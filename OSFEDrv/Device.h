/*++

Module Name:

    device.h

Abstract:

    This file contains the device definitions.

Environment:

    Kernel-mode Driver Framework

--*/
#pragma once

//
// Function to initialize the device and its callbacks
//
NTSTATUS
OSFEDrvCreateDevice(
);

NTSTATUS
OSFEDrvDeleteDevice(
);
