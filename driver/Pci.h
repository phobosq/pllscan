#ifndef PCI_H
#define PCI_H

#include <ntddk.h>
#include "Ioctl.h"

NTSTATUS
	PciScan(
	PVOID Buffer,
	ULONG BufferLength,
	PULONG BytesReturned
	);

NTSTATUS
	PciReadConfig(
	PPCI_READ_CONFIG_REQUEST Request
	);

#endif