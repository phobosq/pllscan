#include <ntddk.h>
#include <wdmsec.h>
#include "plldrv.h"
#include "Ioctl.h"
#include "version.h"
#include "Pci.h"

#pragma comment(lib, "Wdmsec.lib")

static const GUID GUID_DEVCLASS_PLLDRV =
{ 0x9d4d5f10, 0x3c8d, 0x4f94, { 0x8e, 0x8e, 0x3b, 0x62, 0x6d, 0x8b, 0xa6, 0x19 } };

static const UNICODE_STRING g_DefaultDeviceSddl =
RTL_CONSTANT_STRING(L"D:P(A;;GA;;;SY)(A;;GA;;;BA)");

#define DEVICE_NAME      L"\\Device\\PLLDRV"
#define SYMBOLIC_NAME    L"\\DosDevices\\PLLDRV"


PDEVICE_OBJECT g_DeviceObject = NULL;
FAST_MUTEX g_PciConfigMutex;
KMUTEX g_SmbusMutex;


/*
Forward declarations
*/

VOID
	DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	);


NTSTATUS
	DispatchCreate(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);


NTSTATUS
	DispatchClose(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);


NTSTATUS
	DispatchDeviceControl(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	);

NTSTATUS
	ReadPort(
	PPORT_REQUEST Request
	);


NTSTATUS
	WritePort(
	PPORT_WRITE_REQUEST Request
	);

NTSTATUS
	SmbusReadByte(
	PSMBUS_READ_REQUEST Request
	);

NTSTATUS
	SmbusWriteByte(
	PSMBUS_WRITE_REQUEST Request
	);

#include "Port.c"
#include "SMBus.c"


/*
Driver unload
*/

VOID
	DriverUnload(
	IN PDRIVER_OBJECT DriverObject
	)
{
	UNICODE_STRING symLink;

	DbgPrint("PLLDRV: unloading\n");

	RtlInitUnicodeString(
		&symLink,
		SYMBOLIC_NAME
		);

	IoDeleteSymbolicLink(
		&symLink
		);

	if (g_DeviceObject)
	{
		IoDeleteDevice(
			g_DeviceObject
			);
	}
}


/*
Driver entry point
*/

NTSTATUS
	DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath
	)
{
	NTSTATUS status;

	UNICODE_STRING deviceName;
	UNICODE_STRING symbolicName;


	UNREFERENCED_PARAMETER(RegistryPath);


	DbgPrint("PLLDRV: DriverEntry\n");

	ExInitializeFastMutex(&g_PciConfigMutex);
	KeInitializeMutex(&g_SmbusMutex, 0);


	/*
	Set callbacks
	*/

	DriverObject->DriverUnload = DriverUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] =
		DispatchCreate;

	DriverObject->MajorFunction[IRP_MJ_CLOSE] =
		DispatchClose;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
		DispatchDeviceControl;



	/*
	Create device object
	*/

	RtlInitUnicodeString(
		&deviceName,
		DEVICE_NAME
		);


	status = IoCreateDeviceSecure(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&g_DefaultDeviceSddl,
		&GUID_DEVCLASS_PLLDRV,
		&g_DeviceObject
		);


	if (!NT_SUCCESS(status))
	{
		DbgPrint(
			"PLLDRV: IoCreateDeviceSecure failed %08X\n",
			status
			);

		return status;
	}

	DbgPrint(
		"PLLDRV: loading version %s\n",
		PLLDRV_VERSION_STRING
		);

	/*
	Create DOS symbolic link
	so user mode can open:
	\\\\.\\PLLDRV
	*/

	RtlInitUnicodeString(
		&symbolicName,
		SYMBOLIC_NAME
		);


	status = IoCreateSymbolicLink(
		&symbolicName,
		&deviceName
		);


	if (!NT_SUCCESS(status))
	{
		DbgPrint(
			"PLLDRV: IoCreateSymbolicLink failed %08X\n",
			status
			);

		IoDeleteDevice(
			g_DeviceObject
			);

		g_DeviceObject = NULL;

		return status;
	}



	DbgPrint("PLLDRV: loaded successfully\n");


	return STATUS_SUCCESS;
}


/*
IRP_MJ_CREATE
*/

NTSTATUS
	DispatchCreate(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);


	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;


	IoCompleteRequest(
		Irp,
		IO_NO_INCREMENT
		);


	return STATUS_SUCCESS;
}


/*
IRP_MJ_CLOSE
*/

NTSTATUS
	DispatchClose(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	UNREFERENCED_PARAMETER(DeviceObject);


	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;


	IoCompleteRequest(
		Irp,
		IO_NO_INCREMENT
		);


	return STATUS_SUCCESS;
}


/*
IOCTL handler
(na razie pusty)
*/

NTSTATUS
	DispatchDeviceControl(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp
	)
{
	PIO_STACK_LOCATION stack;
	PVOID systemBuffer;
	ULONG inputLength;
	ULONG outputLength;
	ULONG_PTR info;
	NTSTATUS status;

	UNREFERENCED_PARAMETER(DeviceObject);


	stack = IoGetCurrentIrpStackLocation(Irp);
	systemBuffer = Irp->AssociatedIrp.SystemBuffer;
	inputLength = stack->Parameters.DeviceIoControl.InputBufferLength;
	outputLength = stack->Parameters.DeviceIoControl.OutputBufferLength;
	info = 0;
	status = STATUS_INVALID_DEVICE_REQUEST;


	DbgPrint(
		"PLLDRV: IOCTL received %08X expected %08X\n",
		stack->Parameters.DeviceIoControl.IoControlCode,
		IOCTL_PLL_SCAN_PCI
		);


	switch(stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_PLL_TEST:

		DbgPrint("PLLDRV: TEST IOCTL OK\n");

		status = STATUS_SUCCESS;
		info = 0;

		break;
	case IOCTL_PLL_SCAN_PCI:

		if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		{
			status = STATUS_INVALID_DEVICE_STATE;
			break;
		}

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		status = PciScan(
			systemBuffer,
			outputLength,
			&info
			);

		DbgPrint(
			"PLLDRV: returning %lu bytes\n",
			(ULONG)info
			);

		break;
	case IOCTL_PLL_PCI_READ_CONFIG:

		if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		{
			status = STATUS_INVALID_DEVICE_STATE;
			break;
		}

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (inputLength < sizeof(PCI_READ_CONFIG_REQUEST) ||
			outputLength < sizeof(PCI_READ_CONFIG_REQUEST))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = PciReadConfig(
			(PPCI_READ_CONFIG_REQUEST)systemBuffer
			);

		if (NT_SUCCESS(status))
		{
			info = sizeof(PCI_READ_CONFIG_REQUEST);
		}

		break;
	case IOCTL_PLL_READ_PORT:

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (inputLength < sizeof(PORT_REQUEST) ||
			outputLength < sizeof(PORT_REQUEST))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = ReadPort(
			(PPORT_REQUEST)
			systemBuffer
			);

		if (NT_SUCCESS(status))
		{
			info = sizeof(PORT_REQUEST);
		}

		break;
	case IOCTL_PLL_WRITE_PORT:

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (inputLength < sizeof(PORT_WRITE_REQUEST))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = WritePort(
			(PPORT_WRITE_REQUEST)
			systemBuffer
			);
		info = 0;

		break;
	case IOCTL_PLL_SMBUS_READ_BYTE:

		if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		{
			status = STATUS_INVALID_DEVICE_STATE;
			break;
		}

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (inputLength < sizeof(SMBUS_READ_REQUEST) ||
			outputLength < sizeof(SMBUS_READ_REQUEST))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = SmbusReadByte(
			(PSMBUS_READ_REQUEST)
			systemBuffer
			);

		if (NT_SUCCESS(status))
		{
			info = sizeof(SMBUS_READ_REQUEST);
		}

		break;
	case IOCTL_PLL_SMBUS_WRITE_BYTE:

		if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		{
			status = STATUS_INVALID_DEVICE_STATE;
			break;
		}

		if (systemBuffer == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (inputLength < sizeof(SMBUS_WRITE_REQUEST))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = SmbusWriteByte(
			(PSMBUS_WRITE_REQUEST)
			systemBuffer
			);
		info = 0;

		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		info = 0;
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;

	IoCompleteRequest(
		Irp,
		IO_NO_INCREMENT
		);


	return Irp->IoStatus.Status;
}