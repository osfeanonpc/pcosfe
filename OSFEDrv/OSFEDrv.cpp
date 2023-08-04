#include "pch.h"
#include "Device.h"
#include "OSFEDrvPublic.h"
#include "OSFEDrv.h"
#define OSFE_KERNEL
#include "../OSFEpsDrv/PSPublic.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif


OSFEContext OSFEData;


/**
 * \brief Cleans up the driver state.
 */
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS OSFEDriverCleanup(
    VOID
)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTATUS functionRetStatus = STATUS_FAIL_CHECK;

    do {

        if (OSFEData.DriverSetupState[dStatePSInitCleanupRequired]) {
            MSUnload();
            PRINT("Process monitoring driver unloaded\n");
            OSFEData.DriverSetupState[dStatePSInitCleanupRequired] = false;
            if (NT_SUCCESS(status))
                status = STATUS_SUCCESS;
        }

        if (OSFEData.DriverSetupState[dStateDeviceCreationCleanupRequired]) {
            functionRetStatus = OSFEDrvDeleteDevice();
            PRINT("Driver device object and symlink deleted\n");
            OSFEData.DriverSetupState[dStateDeviceCreationCleanupRequired] = false;
            if (NT_SUCCESS(status))
                status = functionRetStatus;
        }

    } while (false);

    if (NT_SUCCESS(status)) {
        PRINT("Clean-up done successfully\n");
    }

    return status;
}


/**
 * \brief Driver unload routine.
 *
 * \param[in] DriverObject Driver object of this driver.
 */
_Function_class_(DRIVER_UNLOAD)
_IRQL_requires_(PASSIVE_LEVEL)
_IRQL_requires_same_
VOID DriverUnload(
    _In_ PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER(DriverObject);
    NTSTATUS status;

    PRINT("Driver Unloading ...\n");

    //
    //  Calling cleanup
    //
    status = OSFEDriverCleanup();

    //
    //  Unloading done safely, ready to return
    //
    PRINT("Driver Unloaded successfully\n");
}


NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

    How it works? initially it sets the status to success. as long as status is success,
    it advances and do other stuff, other than that it will skip and at the clean up,
    it will revert all those changes back to return safely.

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise.

--*/
{
    NTSTATUS status = STATUS_FAILED_DRIVER_ENTRY;

    RtlZeroMemory(&OSFEData, sizeof(OSFEData));
    RtlFillMemory(&OSFEData.DriverSetupState, sizeof(OSFEData.DriverSetupState), false);

    do {
        //
        // Set proper driver context
        //
        OSFEData.DriverObject = DriverObject;
        status = 0;
        if (!NT_SUCCESS(status))
            break;
        RtlCopyUnicodeString(&OSFEData.RegistryPath, RegistryPath);
        

        // 
        // Create device for driver
        // 
        OSFEData.DriverSetupState[dStateDeviceCreationCleanupRequired] = true;
        status = OSFEDrvCreateDevice();
        if (!NT_SUCCESS(status))
            break;


        OSFEData.DriverSetupState[dStatePSInitCleanupRequired] = true;
        status = OSFEProcessInformerInit (&OSFEData.RegistryPath);
        if (!NT_SUCCESS(status))
            break;

    }
    while (false);

    // 
    // Call cleanup if anything failed
    //
    if (!NT_SUCCESS(status))
        OSFEDriverCleanup();

    
    if (NT_SUCCESS(status)) {
        PRINT("Driver loaded successfully\n");
    }
    else {
        PRINT("Driver failed to load\n");
    }
    return status;
}