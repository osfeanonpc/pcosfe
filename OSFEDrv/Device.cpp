/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/
#include "pch.h"
#include "Device.h"
#include "OSFEDrvPublic.h"
#include "OSFEDrv.h"
#include "../OSFEpsDrv/PSPublic.h"


NTSTATUS
OSFEDrvCreateDevice (
)
/*++

Routine Description:

    Worker routine called to create a device and its software resources.
    Will do a self rollback on all failure statuses

Arguments:

    PDRIVER_OBJECT:
        Driver Object for creating the device

    PUNICODE_STRING:
        Registry path for driver service

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status = STATUS_FAILED_DRIVER_ENTRY;
    bool symLinkCreated = false;
    OSFEData.DeviceObject = nullptr;
    UNICODE_STRING devName = RTL_CONSTANT_STRING(OSFEDriverDeviceName);
    UNICODE_STRING symLink = RTL_CONSTANT_STRING(OSFEDriverSymLinkName);

    PRINT("Device and symlink is creating...\n");
    do {
        status = IoCreateDeviceSecure(OSFEData.DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, TRUE, &SDDL_DEVOBJ_SYS_ALL_ADM_ALL, nullptr, &OSFEData.DeviceObject);
        if (!NT_SUCCESS(status)) {
            PRINT("failed to create device (0x%08X)\n", status);
            break;
        }
        OSFEData.DeviceObject->Flags |= DO_DIRECT_IO;

        status = IoCreateSymbolicLink(&symLink, &devName);
        if (!NT_SUCCESS(status)) {
            PRINT("failed to create sym link (0x%08X)\n", status);
            break;
        }
        symLinkCreated = true;

    } while (false);
    
    if (!NT_SUCCESS(status)) {
        if (symLinkCreated)
            IoDeleteSymbolicLink(&symLink);
        if (OSFEData.DeviceObject)
            IoDeleteDevice(OSFEData.DeviceObject);
    }
    else {
        PRINT("Device and symlink created\n");
    }

    OSFEData.DriverObject->MajorFunction[IRP_MJ_READ] = MSRead;
    OSFEData.DriverObject->DriverUnload = DriverUnload;
    OSFEData.DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MSDeviceControl;
    OSFEData.DriverObject->MajorFunction[IRP_MJ_CREATE] = OSFEData.DriverObject->MajorFunction[IRP_MJ_CLOSE] = MSCreateClose;

    if (NT_SUCCESS(status))
        OSFEData.DriverSetupState[dStateDeviceCreationCleanupRequired] = true;

    return status;
}


NTSTATUS
OSFEDrvDeleteDevice (
)
/*++

Routine Description:

    Worker routine called to delete a device and its symlink.
    Will do a self rollback on all failure statuses

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS status = STATUS_DRIVER_FAILED_PRIOR_UNLOAD;
    PRINT("Device and symlink is deleteing...\n");

    UNICODE_STRING symLink = RTL_CONSTANT_STRING(OSFEDriverSymLinkName);
    status = IoDeleteSymbolicLink(&symLink);
    IoDeleteDevice(OSFEData.DriverObject->DeviceObject);
    OSFEData.DriverSetupState[dStateDeviceCreationCleanupRequired] = false;
    PRINT("Device and symlink deleted\n");

    return status;
}
