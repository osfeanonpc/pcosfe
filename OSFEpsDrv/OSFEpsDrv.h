#pragma once
#include "pch.h"
#include "PSPublic.h"

//
// Macro Functions
//
#ifdef _DEBUG
#ifndef DEBUG_PREFIX
#define DEBUG_PREFIX "[DBG OSFE_Ps_Drv]: "
#define PRINT(_x_, ...) DbgPrint(DEBUG_PREFIX _x_, ##__VA_ARGS__);
#endif
#else
#define PRINT(_x_, ...)
#endif

#ifndef DRIVER_TAG
#define DRIVER_TAG 'OEps'
#endif

template<typename T>
struct FullItem {
	LIST_ENTRY Entry;
	T Data;
};


NTSTATUS CompleteRequest(
    PIRP Irp,
    NTSTATUS status = STATUS_SUCCESS,
    ULONG_PTR info = 0
);

void OnProcessNotify(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
);

void OnThreadNotify(
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ BOOLEAN Create
);

void OnImageLoadNotify(
    _In_opt_ PUNICODE_STRING FullImageName,
    _In_ HANDLE ProcessId,
    _In_ PIMAGE_INFO ImageInfo
);


NTSTATUS InitKG(
);

extern "C" POBJECT_TYPE * IoDeviceObjectType;

extern "C" POBJECT_TYPE * LpcPortObjectType;

extern "C" POBJECT_TYPE * IoDriverObjectType;

extern "C" POBJECT_TYPE * ExWindowStationObjectType;

extern "C" POBJECT_TYPE * MmSectionObjectType;

extern "C" POBJECT_TYPE * ExTimerObjectType;

extern "C" POBJECT_TYPE * PsPartitionType;

extern "C" NTSTATUS ZwOpenThread(
	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId);

extern "C" NTSTATUS NTAPI ZwQueryInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength
);

extern "C" NTSTATUS NTAPI ZwOpenSession(
	_Out_ PHANDLE SessionHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes);

extern "C" NTSTATUS ObOpenObjectByName(
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ POBJECT_TYPE ObjectType,
	_In_ KPROCESSOR_MODE AccessMode,
	_Inout_opt_ PACCESS_STATE AccessState,
	_In_opt_ ACCESS_MASK DesiredAccess,
	_Inout_opt_ PVOID ParseContext,
	_Out_ PHANDLE Handle);


enum class ObjectType {
	None = 0,
	Session,
	SymbolicLink,
	Mutant,
};
