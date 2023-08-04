#pragma once

#ifdef OSFE_KERNEL
#include <ntifs.h>
#include <wdmsec.h>
#endif

#include <ntdef.h>

const int MAX_PATH_LENGHT_PS = 512;

enum class ItemType : short {
	None,
	ProcessCreate,
	ProcessExit,
	ThreadCreate,
	ThreadExit,
	ImageLoad
};

struct ItemHeader {
	ItemType Type;
	USHORT Size;
	LARGE_INTEGER Time;
	LARGE_INTEGER QPCTimeForGSeq;
	ULONGLONG DriverSequenceNumber;
	WCHAR ProcessImageFullPath[MAX_PATH_LENGHT_PS + 1];
};

struct ProcessExitInfo : ItemHeader {
	ULONG ProcessId;
	ULONG ExitCode;
};

struct ProcessCreateInfo : ItemHeader {
	ULONG ProcessId;
	ULONG ParentProcessId;
	//LARGE_INTEGER ParentProcessCreationTime;
	ULONG CreatingThreadId;
	ULONG CreatingProcessId;
	USHORT CommandLineLength;
	WCHAR CommandLine[1];
};

struct ThreadCreateInfo : ItemHeader {
	ULONG ThreadId;
	ULONG ProcessId;
};

struct ThreadExitInfo : ThreadCreateInfo {
	ULONG ExitCode;
};

struct ImageLoadInfo : ItemHeader {
	ULONG ProcessId;

/*	union {
		ULONG Properties;
		struct {
			ULONG ImageAddressingMode : 8;  // Code addressing mode
			ULONG SystemModeImage : 1;  // System mode image
			ULONG ImageMappedToAllPids : 1;  // Image mapped into all processes
			ULONG ExtendedInfoPresent : 1;  // IMAGE_INFO_EX available
			ULONG MachineTypeMismatch : 1;  // Architecture type mismatch
			ULONG ImageSignatureLevel : 4;  // Signature level
			ULONG ImageSignatureType : 3;  // Signature type
			ULONG ImagePartialMap : 1;  // Nonzero if entire image is not mapped
			ULONG Reserved : 12;
		};
	};
*/
	ULONG64 ImageBase;
	ULONG ImageSelector;
	SIZE_T ImageSize;
	ULONG ImageSectionNumber;

	WCHAR ImageFileName[MAX_PATH_LENGHT_PS + 1];
};


#define _DEVICE 0x8088
#define K_VERSION 0x201

#define IOCTL_GET_VERSION				CTL_CODE(_DEVICE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_OBJECT_BY_ADDRESS		CTL_CODE(_DEVICE, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DUP_HANDLE					CTL_CODE(_DEVICE, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_OBJECT_BY_NAME		CTL_CODE(_DEVICE, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_PROCESS				CTL_CODE(_DEVICE, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_THREAD				CTL_CODE(_DEVICE, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OPEN_PROCESS_TOKEN			CTL_CODE(_DEVICE, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

#pragma pack(push, 1)

struct OpenObjectByAddressData {
	void* Address;
	ACCESS_MASK Access;
};

struct OpenObjectByNameData {
	ACCESS_MASK Access;
	USHORT TypeIndex;
	WCHAR Name[1];
};

struct DupHandleData {
	ULONG SourceHandle;
	ULONG SourcePid;
	ACCESS_MASK AccessMask;
	ULONG Flags;
};

struct OpenProcessThreadData {
	ULONG Id;
	ACCESS_MASK AccessMask;
};

#ifdef OSFE_KERNEL

NTSTATUS OSFEProcessInformerInit(
	PUNICODE_STRING RegistryPath
);

void MSUnload(
);

NTSTATUS MSCreateClose(
	PDEVICE_OBJECT,
	PIRP Irp
);

NTSTATUS MSRead(
	PDEVICE_OBJECT,
	PIRP Irp
);

NTSTATUS MSDeviceControl(
	PDEVICE_OBJECT,
	PIRP Irp
);

#endif


#pragma pack(pop)
