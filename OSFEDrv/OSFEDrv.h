/*++

Module Name:

    driver.h

Abstract:

    This file contains the driver definitions.

Environment:

    Kernel-mode Driver Framework

--*/
#pragma once
#include "device.h"
#include "OSFEDrvPublic.h"

//
// Macro Functions
//
#ifdef _DEBUG
#ifndef DEBUG_PREFIX
#define DEBUG_PREFIX "[DBG OSFEDrv]: "
#define PRINT(_x_, ...) DbgPrint(DEBUG_PREFIX _x_, ##__VA_ARGS__);
#endif
#else
#define PRINT(_x_, ...)
#endif

#define DRIVER_TAG 'OSFE'


EXTERN_C_START
//
// Driver Entry Declaration
//
NTSTATUS
DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING);

EXTERN_C_END


_Function_class_(DRIVER_UNLOAD)
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
);

NTSTATUS OSFEDriverCleanup(
    VOID
);


extern NTSTATUS OSFEProcessInformerInit(
    PUNICODE_STRING RegistryPath
);

extern void MSUnload(
);

extern NTSTATUS MSCreateClose(
    PDEVICE_OBJECT,
    PIRP Irp
);

extern NTSTATUS MSRead(
    PDEVICE_OBJECT,
    PIRP Irp
);

extern NTSTATUS MSDeviceControl(
    PDEVICE_OBJECT,
    PIRP Irp
);