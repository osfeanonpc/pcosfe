#include "pch.h"
#include "../inc/defines.h"
#include "OSFEpsDrv.h"
#include "DS.h"

DSHolder g_State;
__volatile LONG64 LogSequenceNumber;

namespace NT {
	typedef struct _OBJECT_TYPE_INFORMATION {
		UNICODE_STRING TypeName;
		ULONG TotalNumberOfObjects;
		ULONG TotalNumberOfHandles;
		ULONG TotalPagedPoolUsage;
		ULONG TotalNonPagedPoolUsage;
		ULONG TotalNamePoolUsage;
		ULONG TotalHandleTableUsage;
		ULONG HighWaterNumberOfObjects;
		ULONG HighWaterNumberOfHandles;
		ULONG HighWaterPagedPoolUsage;
		ULONG HighWaterNonPagedPoolUsage;
		ULONG HighWaterNamePoolUsage;
		ULONG HighWaterHandleTableUsage;
		ULONG InvalidAttributes;
		GENERIC_MAPPING GenericMapping;
		ULONG ValidAccessMask;
		BOOLEAN SecurityRequired;
		BOOLEAN MaintainHandleCount;
		UCHAR TypeIndex;
		CHAR ReservedByte;
		ULONG PoolType;
		ULONG DefaultPagedPoolCharge;
		ULONG DefaultNonPagedPoolCharge;
	} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

	enum class ObjectInformationClass {
		BasicInformation,
		NameInformation,
		TypeInformation,
		TypesInformation,
		HandleFlagInformation,
		SessionInformation,
		SessionObjectInformation,
	} OBJECT_INFORMATION_CLASS;

	typedef struct _OBJECT_TYPES_INFORMATION {
		ULONG NumberOfTypes;
		OBJECT_TYPE_INFORMATION TypeInformation[1];
	} OBJECT_TYPES_INFORMATION, * POBJECT_TYPES_INFORMATION;
}

POBJECT_TYPE g_ObjectTypes[256];
ULONG g_NumberOfTypes;



NTSTATUS
OSFEProcessInformerInit (PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	auto status = STATUS_SUCCESS;
	LogSequenceNumber = 0;

	bool processCallbacks = false, threadCallbacks = false;
	PRINT("Starting to load...\n");

	LONG maxCounts = 0;
	status = 0;

	if (NT_SUCCESS(status))
		g_State.init(maxCounts);
	else
		return status;

	do {
		status = PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, FALSE);
		if (!NT_SUCCESS(status)) {
			PRINT("failed to register process callback(0x % 08X)\n", status);
			break;
		}
		processCallbacks = true;

		status = PsSetCreateThreadNotifyRoutine(OnThreadNotify);
		if (!NT_SUCCESS(status)) {
			PRINT("failed to set thread callback (status=%08X)\n", status);
			break;
		}
		threadCallbacks = true;

		status = PsSetLoadImageNotifyRoutine(OnImageLoadNotify);
		if (!NT_SUCCESS(status)) {
			PRINT("failed to set image load callback (status=%08X)\n", status);
			break;
		}

		status = InitKG();
		if (!NT_SUCCESS(status)) {
			PRINT("Warning: Failed to init globals (0x%X)\n", status);
			break;
		}

	} while (false);

	if (!NT_SUCCESS(status)) {
		if (threadCallbacks)
			PsRemoveCreateThreadNotifyRoutine(OnThreadNotify);
		if (processCallbacks)
			PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, TRUE);
		return status;
	}

	PRINT("Loaded successfully\n");
	return status;
}

NTSTATUS MSCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	return CompleteRequest(Irp);
}

NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR info) {
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

void MSUnload() {
	LIST_ENTRY* entry;
	while ((entry = g_State.deleteRecord()) != nullptr)
		ExFreePool(CONTAINING_RECORD(entry, FullItem<ItemHeader>, Entry));

	PRINT("Starting to unloading...\n");
	PsRemoveCreateThreadNotifyRoutine(OnThreadNotify);
	PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, TRUE);
	PsRemoveLoadImageNotifyRoutine(OnImageLoadNotify);
	PRINT("Unloaded successfully\n");
}

NTSTATUS MSRead(PDEVICE_OBJECT, PIRP Irp) {
	auto irpSp = IoGetCurrentIrpStackLocation(Irp);
	auto len = irpSp->Parameters.Read.Length;
	auto status = STATUS_SUCCESS;
	ULONG bytes = 0;
	NT_ASSERT(Irp->MdlAddress);

	auto buffer = (PUCHAR)MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	if (!buffer) {
		status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else {
		while (true) {
			auto entry = g_State.deleteRecord();
			if (entry == nullptr)
				break;

			auto info = CONTAINING_RECORD(entry, FullItem<ItemHeader>, Entry);
			auto size = info->Data.Size;
			if (len < size) {
				g_State.insertHeadRecord(entry);
				break;
			}
			memcpy(buffer, &info->Data, size);
			len -= size;
			buffer += size;
			bytes += size;
			ExFreePool(info);
		}
	}
	return CompleteRequest(Irp, status, bytes);
}

_Use_decl_annotations_
void OnProcessNotify(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo) {
	if (CreateInfo) {
		//
		// process created
		//
		USHORT allocSize = sizeof(FullItem<ProcessCreateInfo>);
		USHORT commandLineSize = 0;
		if (CreateInfo->CommandLine) {
			commandLineSize = CreateInfo->CommandLine->Length;
			allocSize += commandLineSize;
		}
		auto info = (FullItem<ProcessCreateInfo>*)ExAllocatePool2(POOL_FLAG_PAGED, allocSize, DRIVER_TAG);
		if (info == nullptr) {
			PRINT("failed allocation\n");
			return;
		}

		auto& item = info->Data;
		KeQuerySystemTimePrecise(&item.Time);
		item.QPCTimeForGSeq = KeQueryPerformanceCounter(NULL);  // Making a unique timer for making global sequence number in user app
		item.DriverSequenceNumber = InterlockedIncrement64(&LogSequenceNumber);
		NTSTATUS status = 0;
		if (!NT_SUCCESS(status)) {
			PRINT("###### WARNING ######\t\tGetProcessImagePath: status=%X\n", status);
			_wcsnset(item.ProcessImageFullPath, 0, MAX_PATH_LENGHT_PS);
		}
		item.Type = ItemType::ProcessCreate;
		item.Size = sizeof(ProcessCreateInfo) + commandLineSize;
		item.ProcessId = HandleToULong(ProcessId);
		item.ParentProcessId = HandleToULong(CreateInfo->ParentProcessId);
		item.CreatingProcessId = HandleToULong(CreateInfo->CreatingThreadId.UniqueProcess);
		item.CreatingThreadId = HandleToULong(CreateInfo->CreatingThreadId.UniqueThread);

		if (commandLineSize > 0) {
			memcpy(item.CommandLine, CreateInfo->CommandLine->Buffer, commandLineSize);
			item.CommandLineLength = commandLineSize / sizeof(WCHAR);
		}
		else {
			item.CommandLineLength = 0;
		}
		g_State.insertRecord(&info->Entry);
	}
	else {
		//
		// process exited
		//
		auto info = (FullItem<ProcessExitInfo>*)ExAllocatePool2(POOL_FLAG_PAGED, sizeof(FullItem<ProcessExitInfo>), DRIVER_TAG);
		if (info == nullptr) {
			PRINT("failed allocation\n");
			return;
		}

		auto& item = info->Data;
		KeQuerySystemTimePrecise(&item.Time);
		item.QPCTimeForGSeq = KeQueryPerformanceCounter(NULL);  // Making a unique timer for making global sequence number in user app
		item.DriverSequenceNumber = InterlockedIncrement64(&LogSequenceNumber);
		NTSTATUS status = 0;
		if (!NT_SUCCESS(status)) {
			PRINT("###### WARNING ######\t\tGetProcessImagePath: status=%X\n", status);
			_wcsnset(item.ProcessImageFullPath, 0, MAX_PATH_LENGHT_PS);
		}
		item.Type = ItemType::ProcessExit;
		item.ProcessId = HandleToULong(ProcessId);
		item.Size = sizeof(ProcessExitInfo);
		item.ExitCode = PsGetProcessExitStatus(Process);
		
		g_State.insertRecord(&info->Entry);
	}
}

_Use_decl_annotations_
void OnThreadNotify(HANDLE ProcessId, HANDLE ThreadId, BOOLEAN Create) {
	auto size = Create ? sizeof(FullItem<ThreadCreateInfo>) : sizeof(FullItem<ThreadExitInfo>);
	auto info = (FullItem<ThreadExitInfo>*)ExAllocatePool2(POOL_FLAG_PAGED, size, DRIVER_TAG);
	if (info == nullptr) {
		PRINT("Failed to allocate memory\n");
		return;
	}
	auto& item = info->Data;
	KeQuerySystemTimePrecise(&item.Time);
	item.QPCTimeForGSeq = KeQueryPerformanceCounter(NULL);  // Making a unique timer for making global sequence number in user app
	item.DriverSequenceNumber = InterlockedIncrement64(&LogSequenceNumber);
	NTSTATUS status = 0;
	if (!NT_SUCCESS(status)) {
		PRINT("###### WARNING ######\t\tGetProcessImagePath: status=%X\n", status);
		_wcsnset(item.ProcessImageFullPath, 0, MAX_PATH_LENGHT_PS);
	}
	item.Size = Create ? sizeof(ThreadCreateInfo) : sizeof(ThreadExitInfo);
	item.Type = Create ? ItemType::ThreadCreate : ItemType::ThreadExit;
	item.ProcessId = HandleToULong(ProcessId);
	item.ThreadId = HandleToULong(ThreadId);
	if (!Create) {
		PETHREAD thread;
		if (NT_SUCCESS(PsLookupThreadByThreadId(ThreadId, &thread))) {
			item.ExitCode = PsGetThreadExitStatus(thread);
			ObDereferenceObjectDeferDelete(thread);
		}
	}
	g_State.insertRecord(&info->Entry);
}

_Use_decl_annotations_
void OnImageLoadNotify(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo) {
	UNREFERENCED_PARAMETER(FullImageName);

	if (ProcessId == nullptr) {
		return;
	}

	auto size = sizeof(FullItem<ImageLoadInfo>);
	auto info = (FullItem<ImageLoadInfo>*)ExAllocatePool2(POOL_FLAG_PAGED, size, DRIVER_TAG);
	if (info == nullptr) {
		PRINT("Failed to allocate memory\n");
		return;
	}

	auto& item = info->Data;
	KeQuerySystemTimePrecise(&item.Time);
	item.QPCTimeForGSeq.QuadPart = KeQueryPerformanceCounter(NULL).QuadPart;  // Making a unique timer for making global sequence number in user app
	item.DriverSequenceNumber = InterlockedIncrement64(&LogSequenceNumber);
	NTSTATUS status = 0;
	if (!NT_SUCCESS(status)) {
		PRINT("###### WARNING ######\t\tGetProcessImagePath: status=%X\n", status);
		_wcsnset(item.ProcessImageFullPath, 0, MAX_PATH_LENGHT_PS);
	}
	item.Size = sizeof(item);
	item.Type = ItemType::ImageLoad;
	item.ProcessId = HandleToULong(ProcessId);
	item.ImageBase = (ULONG64)ImageInfo->ImageBase;
	item.ImageSelector = ImageInfo->ImageSelector;
	item.ImageSize = ImageInfo->ImageSize;
	item.ImageSectionNumber = ImageInfo->ImageSectionNumber;

	g_State.insertRecord(&info->Entry);
}

NTSTATUS MSDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	const auto& dic = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl;
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG len = 0;
	auto sysBuffer = Irp->AssociatedIrp.SystemBuffer;

	switch (dic.IoControlCode) {
	case IOCTL_GET_VERSION:
		if (sysBuffer == nullptr || dic.OutputBufferLength < sizeof(ULONG)) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		*(ULONG*)sysBuffer = K_VERSION;
		len = sizeof(ULONG);
		status = STATUS_SUCCESS;
		break;
	}
	return CompleteRequest(Irp, status, len);
}

NTSTATUS InitKG() {
	ULONG size = 1 << 14;
	auto types = (NT::POBJECT_TYPES_INFORMATION)ExAllocatePool2(POOL_FLAG_PAGED | POOL_FLAG_UNINITIALIZED, size, DRIVER_TAG);
	if (!types)
		return STATUS_NO_MEMORY;

	auto status = ZwQueryObject(nullptr, (OBJECT_INFORMATION_CLASS)NT::ObjectInformationClass::TypesInformation, types, size, nullptr);

	if (NT_SUCCESS(status)) {
		const struct {
			PCWSTR Name;
			POBJECT_TYPE Type;
			PVOID Function;
		} objectTypes[] = {
			{ L"Process", *PsProcessType },
			{ L"Thread", *PsThreadType },
			{ L"Job", *PsJobType },
			{ L"Event", *ExEventObjectType },
			{ L"Semaphore", *ExSemaphoreObjectType },
			{ L"File", *IoFileObjectType },
			{ L"Key", *CmKeyObjectType },
			{ L"Token", *SeTokenObjectType },
			{ L"Device", *IoDeviceObjectType },
			{ L"Driver", *IoDriverObjectType },
			{ L"Section", *MmSectionObjectType },
			{ L"Desktop", *ExDesktopObjectType },
			{ L"WindowStation", *ExWindowStationObjectType },
			{ L"ALPC Port", *LpcPortObjectType },
			{ L"Timer", *ExTimerObjectType },
			{ L"Partition", *PsPartitionType },
			{ L"Session", POBJECT_TYPE(ObjectType::Session) },
			{ L"SymbolicLink", POBJECT_TYPE(ObjectType::SymbolicLink) },
		};

		g_NumberOfTypes = types->NumberOfTypes;
		auto type = &types->TypeInformation[0];
		for (ULONG i = 0; i < types->NumberOfTypes; i++) {
			for (auto& ot : objectTypes) {
				if (_wcsnicmp(ot.Name, type->TypeName.Buffer, wcslen(ot.Name)) == 0) {
					g_ObjectTypes[type->TypeIndex] = ot.Type;
					break;
				}
			}
			auto temp = (PUCHAR)type + sizeof(NT::OBJECT_TYPE_INFORMATION) + type->TypeName.MaximumLength;
			temp += sizeof(PVOID) - 1;
			type = reinterpret_cast<NT::OBJECT_TYPE_INFORMATION*>((ULONG_PTR)temp / sizeof(PVOID) * sizeof(PVOID));
		}
	}

	ExFreePool(types);
	return status;
}