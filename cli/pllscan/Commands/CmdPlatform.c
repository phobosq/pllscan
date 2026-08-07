#include <stdio.h>

#include "Commands.h"
#include "Platform.h"

int
	CmdPlatform(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);


	if (!PlatformDetect(Context))
	{
		printf(
			"Platform detection failed\n"
			);

		return 1;
	}


	printf("\n");
	printf("Platform information\n");
	printf("--------------------\n");

	printf(
		"Southbridge : %s\n",
		Context->Platform.SouthbridgeName
		);


	printf(
		"Vendor ID   : %04X\n",
		Context->Platform.SouthbridgeVendor
		);


	printf(
		"Device ID   : %04X\n",
		Context->Platform.SouthbridgeDevice
		);

	printf(
		"SMBus Base  : %04X\n",
		Context->Platform.SmbusBase    );


	return 0;
}