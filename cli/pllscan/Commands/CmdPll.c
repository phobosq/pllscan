#include <stdio.h>
#include <string.h>

#include "Commands.h"
#include "Smbus.h"
#include "PllChipDb.h"
#include "Util.h"
#include "SmbusRegs.h"


#define MAX_DUMP_REGS  64


static
	BOOL
	ReadRegisters(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR *Buf,
	UCHAR Count
	)
{
	UCHAR i;
	UCHAR status;

	for (i = 0; i < Count; i++)
	{
		if (!SmbusReadByte(Context, Address, i, &Buf[i], &status))
			return FALSE;

		if (status & 0xF8)
			return FALSE;
	}

	return TRUE;
}


static
	void
	PrintDumpTable(
	const UCHAR *Buf,
	UCHAR Count
	)
{
	UCHAR i;

	printf("Reg  Val\n");
	printf("---  ---\n");

	for (i = 0; i < Count; i++)
	{
		printf(" %02d  %02X", i, Buf[i]);

		/* mark byte boundaries for easier register grouping */
		if ((i & 7) == 7)
			printf("  <");

		printf("\n");
	}
}


int
	CmdPll(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	if (argc >= 3 && !_stricmp(argv[2], "probe"))
	{
		UCHAR addrs[32];
		UCHAR i;
		int addrCount;
		int found = 0;

		PllChipDbGetProbeAddresses(addrs, 32, &addrCount);
		if (addrCount <= 0)
		{
			addrs[0] = 0x69;
			addrCount = 1;
		}

		printf("Probing PLL chip addresses...\n\n");

		for (i = 0; i < (UCHAR)addrCount; i++)
		{
			UCHAR addr = addrs[i];
			UCHAR r0 = 0;
			UCHAR status = 0;
			PPLL_CHIP_DESC chip;
			BOOL responds;

			responds = SmbusReadByte(
				Context,
				addr,
				0,
				&r0,
				&status);

			if (!responds || (status & SMB_ERR_MASK))
				continue;

			found++;

			chip = PllChipMatch(addr, r0);

			if (chip != NULL)
			{
				printf(
					"  addr=%3d (0x%02X)  R0=0x%02X  %s\n",
					addr, addr, r0, chip->Name);

				/* store first match in context */
				if (Context->Platform.PllType == PLL_UNKNOWN)
				{
					Context->Platform.PllType    = chip->Type;
					Context->Platform.PllAddress = addr;
					strncpy(
						Context->Platform.PllName,
						chip->Name,
						sizeof(Context->Platform.PllName) - 1);
				}
			}
			else
			{
				/* no signature match — report raw value, note likely chip */
				printf(
					"  addr=%3d (0x%02X)  R0=0x%02X  (unidentified",
					addr, addr, r0);

				if (addr == 0x69)
					printf(" - likely RTM880N-793 or ICS LPRS47x");

				printf(")\n");
			}
		}

		if (found == 0)
			printf("  No PLL chips found.\n");

		printf("\n");

		return 0;
	}


	if (argc >= 3 && !_stricmp(argv[2], "dump"))
	{
		UCHAR addr;
		UCHAR count;
		UCHAR buf[MAX_DUMP_REGS];
		PPLL_CHIP_DESC chip;
		UCHAR r0 = 0;
		UCHAR status = 0;

		if (argc < 4)
		{
			printf("usage: pllscan pll dump <addr>\n");
			return 1;
		}

		addr  = (UCHAR)ParseInt(argv[3]);
		count = 32;

		/* try to read R0 for chip identification */
		if (SmbusReadByte(Context, addr, 0, &r0, &status) &&
			!(status & SMB_ERR_MASK))
		{
			chip = PllChipMatch(addr, r0);

			if (chip != NULL)
			{
				printf("Chip: %s\n", chip->Name);
				count = chip->RegisterCount;
			}
			else
			{
				printf("Chip: unidentified (addr=%d R0=0x%02X)\n",
					addr, r0);
			}
		}
		else
		{
			printf("error: no response from addr=%d\n", addr);
			return 1;
		}

		printf("Addr: %d (0x%02X)  Regs: %d\n\n",
			addr, addr, count);

		if (!ReadRegisters(Context, addr, buf, count))
		{
			printf("error: register read failed\n");
			return 1;
		}

		PrintDumpTable(buf, count);

		return 0;
	}


	if (argc >= 3 && !_stricmp(argv[2], "decode"))
	{
		UCHAR addr;
		UCHAR buf[MAX_DUMP_REGS];
		UCHAR count;
		UCHAR i;
		PPLL_CHIP_DESC chip;
		UCHAR r0 = 0;
		UCHAR status = 0;

		if (argc < 4)
		{
			printf("usage: pllscan pll decode <addr>\n");
			return 1;
		}

		addr  = (UCHAR)ParseInt(argv[3]);
		count = 32;

		if (!SmbusReadByte(Context, addr, 0, &r0, &status) ||
			(status & 0xF8))
		{
			printf("error: no response from addr=%d\n", addr);
			return 1;
		}

		chip = PllChipMatch(addr, r0);

		if (!ReadRegisters(Context, addr, buf, count))
		{
			printf("error: register read failed\n");
			return 1;
		}

		printf("PLL decode  addr=%d (0x%02X)\n", addr, addr);
		printf("Chip: %s\n\n",
			chip != NULL ? chip->Name : "unidentified");

		for (i = 0; i < count; i++)
		{
			const char *desc = NULL;

			if (chip != NULL)
				desc = PllChipRegDescription(chip->Type, i);

			if (desc != NULL)
				printf("R%02d = 0x%02X  %s\n", i, buf[i], desc);
			else
				printf("R%02d = 0x%02X\n", i, buf[i]);
		}

		printf("\nNote: decode labels come from PllChips.ini RegXXDesc keys.\n");

		return 0;
	}


	printf("usage:\n");
	printf("  pllscan pll probe\n");
	printf("  pllscan pll dump <addr>\n");
	printf("  pllscan pll decode <addr>\n");

	return 1;
}
