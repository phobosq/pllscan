#include "Pci.h"
#include "Ioctl.h"
#include "plldrv.h"


#define MAX_PCI_DEVICES 256


static ULONG PciReadConfig32(
	UCHAR Bus,
	UCHAR Device,
	UCHAR Function,
	UCHAR Offset
	)
{
	ULONG address;
	ULONG value;

	address =
		0x80000000 |
		((ULONG)Bus << 16) |
		((ULONG)Device << 11) |
		((ULONG)Function << 8) |
		(Offset & 0xFC);

	ExAcquireFastMutex(&g_PciConfigMutex);

	WRITE_PORT_ULONG((PULONG)0xCF8, address);

	value = READ_PORT_ULONG((PULONG)0xCFC);

	ExReleaseFastMutex(&g_PciConfigMutex);

	return value;
}


NTSTATUS PciScan(
	PVOID Buffer,
	ULONG BufferLength,
	PULONG BytesReturned
	)
{
	UCHAR bus;
	UCHAR device;
	UCHAR function;

	ULONG count = 0;
	ULONG id;
	USHORT vendor;
	USHORT deviceId;
	ULONG classReg;

	PPCI_DEVICE_INFO output;

	DbgPrint("PLLDRV: PciScan enter\n");

	output = (PPCI_DEVICE_INFO)Buffer;

	if (BufferLength < sizeof(PCI_DEVICE_INFO))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	DbgPrint("PLLDRV: scanning...\n");
	for (bus = 0; bus < 1; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (function = 0; function < 8; function++)
			{
				DbgPrint("PLLDRV: %u:%u.%u\n", bus, device, function);

				id = PciReadConfig32(
					bus,
					device,
					function,
					0
					);

				if ((id & 0xFFFF) == 0xFFFF)
				{
					continue;
				}

				vendor = (USHORT)(id & 0xFFFF);
				deviceId = (USHORT)(id >> 16);

				if (id == 0)
				{
					continue;
				}

				if (count >= (BufferLength / sizeof(PCI_DEVICE_INFO)))
				{
					break;
				}

				classReg = PciReadConfig32(
					bus,
					device,
					function,
					0x08
					);

				RtlZeroMemory(
					&output[count],
					sizeof(PCI_DEVICE_INFO)
					);

				output[count].Bus = bus;
				output[count].Device = device;
				output[count].Function = function;

				output[count].VendorId =
					vendor;

				output[count].DeviceId =
					deviceId;
				output[count].ClassCode =
					(UCHAR)(classReg >> 24);

				output[count].SubClass =
					(UCHAR)(classReg >> 16);


				count++;
			}
		}
	}

	DbgPrint("PLLDRV: found %lu devices\n", count);


	if (BytesReturned) {
		*BytesReturned =
			count * sizeof(PCI_DEVICE_INFO);
	}
	return STATUS_SUCCESS;
}

NTSTATUS
	PciReadConfig(
	PPCI_READ_CONFIG_REQUEST req
	)
{
	req->Value = PciReadConfig32(
		req->Bus,
		req->Device,
		req->Function,
		req->Offset
		);

	return STATUS_SUCCESS;
}