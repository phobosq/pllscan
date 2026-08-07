#include <stdio.h>
#include <Windows.h>

#include "Commands.h"
#include "..\DriverApi.h"
#include "Ioctl.h"
#include "..\Util.h"


static
	void
	PrintPciList(
	PPCI_DEVICE_INFO Devices,
	ULONG Count
	)
{
	ULONG i;


	printf("\n");
	printf("Bus Dev Fn Vendor Device Class Sub\n");
	printf("----------------------------------\n");


	for (i = 0; i < Count; i++)
	{
		printf(
			"%02X  %02X  %02X  %04X   %04X   %02X    %02X\n",
			Devices[i].Bus,
			Devices[i].Device,
			Devices[i].Function,
			Devices[i].VendorId,
			Devices[i].DeviceId,
			Devices[i].ClassCode,
			Devices[i].SubClass            );
	}
}


int
	CmdPci(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	//
	// pllscan pci scan
	//

	if (argc == 3 &&
		!_stricmp(argv[2], "scan"))
	{
		BYTE buffer[4096];

		ULONG bytes;


		if (!DrvPciScan(
			Context->Driver,
			buffer,
			sizeof(buffer),
			&bytes))
		{
			printf(
				"PCI scan failed\n");

			return 1;
		}


		PrintPciList(
			(PPCI_DEVICE_INFO)buffer,
			bytes / sizeof(PCI_DEVICE_INFO)
			);


		return 0;
	}


	//
	// pllscan pci read bus dev fn offset
	//

	if (argc == 7 &&
		!_stricmp(argv[2], "read"))
	{
		ULONG value;


		if (!DrvPciReadConfig(
			Context->Driver,
			(UCHAR)ParseHex(argv[3]),
			(UCHAR)ParseHex(argv[4]),
			(UCHAR)ParseHex(argv[5]),
			(UCHAR)ParseHex(argv[6]),
			&value))
		{
			printf(
				"PCI read failed\n");

			return 1;
		}


		printf(
			"PCI %02X:%02X.%02X offset %02X = %08X\n",
			ParseHex(argv[3]),
			ParseHex(argv[4]),
			ParseHex(argv[5]),
			ParseHex(argv[6]),
			value
			);


		return 0;
	}


	PrintUsage();

	return 1;
}