#include "PllChipDb.h"


/*
Known PLL clock generators.

Signature values marked with SignatureMask = 0 are not yet confirmed from
hardware — run "pll dump <addr>" and compare R0 against the -792 datasheet
to fill in the correct mask/value for the -793.
*/

static PLL_CHIP_DESC g_PllChips[] =
{
	/*
	Realtek RTM880N-793
	Found on AMD 790FX boards (Gigabyte GA-790FXTA-UD5).
	Signature unverified — update SignatureMask/Value after first pll dump.
	*/
	{
		PLL_RTM880N_793,
		"Realtek RTM880N-793",
		0x69,
		32,
		0x00,
		0x00,   /* SignatureMask = 0: skip signature check */
		0x00
	},

	/*
	ICS9LPRS477DGLF
	Found on Intel and some AMD boards.
	*/
	{
		PLL_ICS_LPRS477,
		"ICS LPRS477",
		0x69,
		21,
		0x00,
		0x00,
		0x00
	},

	/*
	ICS9LPRS918 / ICS9LPRS919
	Intel ICH-era boards.
	*/
	{
		PLL_ICS_LPRS918,
		"ICS LPRS918",
		0x69,
		24,
		0x00,
		0x00,
		0x00
	},
	{
		PLL_ICS_LPRS919,
		"ICS LPRS919",
		0x69,
		24,
		0x00,
		0x00,
		0x00
	},

	/* terminator */
	{ PLL_UNKNOWN, NULL, 0, 0, 0, 0, 0 }
};


PPLL_CHIP_DESC
	PllChipMatch(
	UCHAR Address,
	UCHAR R0
	)
{
	PPLL_CHIP_DESC entry;

	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (entry->DefaultAddress != Address)
			continue;

		if (entry->SignatureMask != 0)
		{
			if ((R0 & entry->SignatureMask) != entry->SignatureValue)
				continue;
		}

		return entry;
	}

	return NULL;
}


PPLL_CHIP_DESC
	PllChipByType(
	PLL_CHIP_TYPE Type
	)
{
	PPLL_CHIP_DESC entry;

	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (entry->Type == Type)
			return entry;
	}

	return NULL;
}
