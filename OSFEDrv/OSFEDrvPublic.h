/*++

Module Name:

    public.h

Environment:

    kernel

--*/
#pragma once

#include "pch.h"
#include "../inc/defines.h"

#ifndef dStateSetup
#define dStateSetup

#define dStateDeviceCreationCleanupRequired 1
#define dStatePSInitCleanupRequired 3
#define dStateTotalStatus 32

#endif

typedef struct _OSFEContext {

    PDRIVER_OBJECT  DriverObject;

    UNICODE_STRING RegistryPath;

    PDEVICE_OBJECT DeviceObject;

    //
    //  Variable and lock for maintaining LogRecord sequence numbers.
    //
    __volatile LONG64 LogSequenceNumber;

    //
    // Every success init make this flag true
    // any failed init make this flag false
    // on clean up all success init must do a clean up
    //
    bool DriverSetupState[dStateTotalStatus];

} OSFEContext;
typedef struct _OSFEContext* POSFEContext;

extern OSFEContext OSFEData;
