#include <stdio.h>

#include "Commands.h"
#include "..\DriverApi.h"
#include "..\Util.h"


int
	CmdPort(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	UCHAR value;


	if (argc == 4 &&
		!_stricmp(argv[2], "read"))
	{
		if (!DrvPortRead(
			Context->Driver,
			(USHORT)ParseHex(argv[3]),
			&value))
		{
			printf("port read failed\n");
			return 1;
		}


		printf(
			"%04X = %02X\n",
			ParseHex(argv[3]),
			value);

		return 0;
	}


	if (argc == 5 &&
		!_stricmp(argv[2], "write"))
	{
		if (!DrvPortWrite(
			Context->Driver,
			(USHORT)ParseHex(argv[3]),
			(UCHAR)ParseHex(argv[4])))
		{
			printf("port write failed\n");
			return 1;
		}


		printf("OK\n");

		return 0;
	}


	PrintUsage();

	return 1;
}