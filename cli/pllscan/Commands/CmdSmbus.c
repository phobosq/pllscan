#include <stdio.h>

#include "Commands.h"
#include "DriverApi.h"
#include "Util.h"
#include "Smbus.h"
#include "SmbusRegs.h"


int
	CmdSmbus(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	UCHAR data;
	UCHAR status;


	if (argc == 5 &&
		!_stricmp(argv[2], "read"))
	{
		UCHAR addr;
		UCHAR cmd;
		status = 0xFF;


		addr = (UCHAR)ParseInt(argv[3]);
		cmd  = (UCHAR)ParseInt(argv[4]);

		if (!SmbusReadByte(
			Context,
			addr,
			cmd,
			&data,
			&status))
		{
			printf(
				"SMBus read failed (status=0x%02X)\n",
				status);


			return 1;
		}


		printf(
			"Slave=%02X Cmd=%02X Status=%02X Data=%02X\n",
			addr,
			cmd,
			status,
			data);


		return 0;
	}


	if (argc == 7 &&
		!_stricmp(argv[2], "write") &&
		!_stricmp(argv[6], "--force"))
	{
		UCHAR addr;
		UCHAR cmd;
		UCHAR val;

		addr = (UCHAR)ParseInt(argv[3]);
		cmd  = (UCHAR)ParseInt(argv[4]);
		val  = (UCHAR)ParseInt(argv[5]);

		if (!SmbusWriteByte(Context, addr, cmd, val))
		{
			printf("SMBus write failed\n");
			return 1;
		}

		printf(
			"Slave=%d Cmd=%d Data=%d  OK\n",
			addr,
			cmd,
			val);

		return 0;
	}

	if (argc >= 3 &&
		!_stricmp(argv[2], "write"))
	{
		printf("error: --force flag required for smbus write\n");
		return 1;
	}


	if (argc == 3 &&
		!_stricmp(argv[2], "scan"))
	{
		int addr;

		printf("Scanning SMBus addresses 8..119...\n");

		for (addr = 8; addr <= 119; addr++)
		{
			BOOL found = FALSE;
			const char *how = "";

			if (SmbusQuickRead(Context, (UCHAR)addr))
			{
				found = TRUE;
				how = "quick";
			}
			else if (SmbusQuickWrite(Context, (UCHAR)addr))
			{
				found = TRUE;
				how = "write-quick";
			}
			else
			{
				/* Last resort: try byte read at register 0 */
				UCHAR d, s;

				if (SmbusReadByte(Context, (UCHAR)addr, 0, &d, &s) &&
					!(s & SMB_ERR_MASK))
				{
					found = TRUE;
					how = "byte";
				}
			}

			if (found)
			{
				printf(
					"  addr=%3d (0x%02X)  [%s]\n",
					addr,
					addr,
					how);
			}
		}

		printf("Scan complete.\n");

		return 0;
	}


	PrintUsage();

	return 1;
}