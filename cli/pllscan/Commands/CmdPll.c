#include <stdio.h>
#include <string.h>

#include "Commands.h"
#include "Smbus.h"
#include "PllChipDb.h"
#include "Util.h"
#include "SmbusRegs.h"


/* Addresses to probe in order */
static const UCHAR g_ProbeAddresses[] =
{
	0x69, 0x6A,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F
};

#define PROBE_ADDRESS_COUNT \
	(sizeof(g_ProbeAddresses) / sizeof(g_ProbeAddresses[0]))

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
		UCHAR i;
		int found = 0;

		printf("Probing PLL chip addresses...\n\n");

		for (i = 0; i < PROBE_ADDRESS_COUNT; i++)
		{
			UCHAR addr = g_ProbeAddresses[i];
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

		/*
		Field annotations are based on RTM880N-792 reverse engineering;
		verify each field with the -793 datasheet when available.
		Update this table once pll dump output is correlated against
		a known-good clock configuration.
		*/

		printf("R00 = 0x%02X  chip ID / revision\n",       buf[0]);
		printf("R01 = 0x%02X  [spread spectrum control?]\n",buf[1]);
		printf("R02 = 0x%02X  [PLL control]\n",             buf[2]);
		printf("R03 = 0x%02X  [PLL control]\n",             buf[3]);
		printf("R04 = 0x%02X  [PLL control]\n",             buf[4]);
		printf("R05 = 0x%02X  [HT clock divider?]\n",       buf[5]);
		printf("R06 = 0x%02X  [HT clock divider?]\n",       buf[6]);
		printf("R07 = 0x%02X  [PCIe ref control?]\n",       buf[7]);
		printf("R08 = 0x%02X  [DDR ref control?]\n",        buf[8]);
		printf("R09 = 0x%02X  [spread spectrum enable?]\n", buf[9]);

		if (count > 10)
		{
			UCHAR i;

			printf("\n");

			for (i = 10; i < count; i++)
				printf("R%02d = 0x%02X\n", i, buf[i]);
		}

		printf("\nNote: field names are estimates from -792 reverse engineering.\n");
		printf("      Run on known 200MHz config and compare to confirm.\n");

		return 0;
	}


	printf("usage:\n");
	printf("  pllscan pll probe\n");
	printf("  pllscan pll dump <addr>\n");
	printf("  pllscan pll decode <addr>\n");

	return 1;
}
