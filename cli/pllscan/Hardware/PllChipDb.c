#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "PllChipDb.h"


/* keep one dummy fallback entry; real data is loaded from PllChips.ini */
static PLL_CHIP_DESC g_PllChips[] =
{
	{ PLL_UNKNOWN, "UNKNOWN", NULL, 0, 0, { { 0 } }, 0, 0, 0 }
};

static PLL_CHIP_DESC *g_LoadedPllChips;
static int            g_LoadedPllCount;

static UCHAR g_ProbeAddresses[32];
static int   g_ProbeAddressCount;


static
	PLL_CHIP_TYPE
	MapTypeString(
	const char *Str
	)
{
	char *end;
	unsigned long n;

	if (!Str || Str[0] == '\0')
		return PLL_UNKNOWN;

	/* Allow explicit numeric type IDs in config for full decoupling. */
	n = strtoul(Str, &end, 0);
	if (*end == '\0')
	{
		if (n >= (unsigned long)PLL_RTM880N_793 &&
		    n <= (unsigned long)PLL_ICS_LPRS919)
		{
			return (PLL_CHIP_TYPE)n;
		}

		return PLL_UNKNOWN;
	}

	if (_stricmp(Str, "RTM880N_793") == 0 || _stricmp(Str, "PLL_RTM880N_793") == 0)
		return PLL_RTM880N_793;

	if (_strnicmp(Str, "ICS_LPRS", 8) == 0)
	{
		n = strtoul(Str + 8, &end, 10);
		if (*end == '\0')
		{
			if (n == 477) return PLL_ICS_LPRS477;
			if (n == 918) return PLL_ICS_LPRS918;
			if (n == 919) return PLL_ICS_LPRS919;
		}
	}

	if (_strnicmp(Str, "PLL_ICS_LPRS", 12) == 0)
	{
		n = strtoul(Str + 12, &end, 10);
		if (*end == '\0')
		{
			if (n == 477) return PLL_ICS_LPRS477;
			if (n == 918) return PLL_ICS_LPRS918;
			if (n == 919) return PLL_ICS_LPRS919;
		}
	}

	return PLL_UNKNOWN;
}


static
	void
	BuildProbeAddressList(
	void
	)
{
	int i;
	int j;
	UCHAR addr;
	int exists;

	g_ProbeAddressCount = 0;

	for (i = 0; i < g_LoadedPllCount; i++)
	{
		addr = g_LoadedPllChips[i].DefaultAddress;

		if (addr == 0)
			continue;

		exists = 0;
		for (j = 0; j < g_ProbeAddressCount; j++)
		{
			if (g_ProbeAddresses[j] == addr)
			{
				exists = 1;
				break;
			}
		}

		if (!exists && g_ProbeAddressCount < (int)(sizeof(g_ProbeAddresses)))
			g_ProbeAddresses[g_ProbeAddressCount++] = addr;
	}
}


BOOL
	PllChipDbLoad(
	const char *Path
	)
{
	char sections[4096];
	const char *p;
	int count;
	int i;
	char tmp[256];
	PLL_CHIP_DESC *entries;

	if (GetPrivateProfileSectionNamesA(sections, sizeof(sections), Path) == 0)
		return FALSE;

	count = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
		count++;

	if (count == 0)
		return FALSE;

	entries = (PLL_CHIP_DESC *)calloc(count + 1, sizeof(PLL_CHIP_DESC));
	if (!entries)
		return FALSE;

	i = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
	{
		int r;

		PllChipKeyFromString(p, entries[i].PllKey, sizeof(entries[i].PllKey));
		entries[i].Type = MapTypeString(p);
		if (entries[i].Type == PLL_UNKNOWN)
			continue;

		GetPrivateProfileStringA(p, "Name", p, tmp, sizeof(tmp), Path);
		entries[i].Name = _strdup(tmp);

		GetPrivateProfileStringA(p, "DefaultAddress", "0", tmp, sizeof(tmp), Path);
		entries[i].DefaultAddress = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "RegisterCount", "0", tmp, sizeof(tmp), Path);
		entries[i].RegisterCount = (UCHAR)strtoul(tmp, NULL, 10);

		GetPrivateProfileStringA(p, "SignatureReg", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureReg = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureMask", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureMask = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureValue", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureValue = (UCHAR)strtoul(tmp, NULL, 16);

		for (r = 0; r < 64; r++)
		{
			char key[16];

			sprintf(key, "Reg%02dDesc", r);
			GetPrivateProfileStringA(
				p,
				key,
				"",
				entries[i].RegDescriptions[r],
				sizeof(entries[i].RegDescriptions[r]),
				Path);
		}

		i++;
	}

	PllChipDbFree();

	g_LoadedPllChips = entries;
	g_LoadedPllCount = i;

	BuildProbeAddressList();

	return TRUE;
}


void
	PllChipDbFree(
	void
	)
{
	int i;

	if (!g_LoadedPllChips)
	{
		g_ProbeAddressCount = 0;
		return;
	}

	for (i = 0; i < g_LoadedPllCount; i++)
		free((void *)g_LoadedPllChips[i].Name);

	free(g_LoadedPllChips);

	g_LoadedPllChips = NULL;
	g_LoadedPllCount = 0;
	g_ProbeAddressCount = 0;
}


void
	PllChipDbGetProbeAddresses(
	UCHAR *Out,
	int MaxCount,
	int *CountOut
	)
{
	int i;
	int n;

	if (!Out || !CountOut || MaxCount <= 0)
		return;

	n = g_ProbeAddressCount < MaxCount ? g_ProbeAddressCount : MaxCount;

	for (i = 0; i < n; i++)
		Out[i] = g_ProbeAddresses[i];

	*CountOut = n;
}


BOOL
	PllChipTypeFromString(
	const char *Str,
	PLL_CHIP_TYPE *TypeOut
	)
{
	PPLL_CHIP_DESC chip;
	PLL_CHIP_TYPE type;
	char key[32];

	if (!Str || !TypeOut || Str[0] == '\0')
		return FALSE;

	if (PllChipKeyFromString(Str, key, sizeof(key)))
	{
		chip = PllChipByKey(key);
		if (chip)
		{
			*TypeOut = chip->Type;
			return TRUE;
		}
	}

	type = MapTypeString(Str);
	if (type != PLL_UNKNOWN)
	{
		*TypeOut = type;
		return TRUE;
	}

	return FALSE;
}


BOOL
	PllChipKeyFromString(
	const char *Str,
	char *KeyOut,
	int KeyOutLen
	)
{
	if (!Str || !KeyOut || KeyOutLen <= 0 || Str[0] == '\0')
		return FALSE;

	strncpy(KeyOut, Str, (size_t)KeyOutLen - 1);
	KeyOut[KeyOutLen - 1] = '\0';
	return TRUE;
}


const char *
	PllChipTypeToString(
	PLL_CHIP_TYPE Type
	)
{
	PPLL_CHIP_DESC chip = PllChipByType(Type);

	return chip ? chip->PllKey : "UNKNOWN";
}


PPLL_CHIP_DESC
	PllChipByKey(
	const char *Key
	)
{
	PPLL_CHIP_DESC entry;
	int i;

	if (!Key || Key[0] == '\0')
		return NULL;

	if (g_LoadedPllChips)
	{
		for (i = 0; i < g_LoadedPllCount; i++)
		{
			entry = &g_LoadedPllChips[i];
			if (_stricmp(entry->PllKey, Key) == 0)
				return entry;
		}
	}

	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (_stricmp(entry->PllKey, Key) == 0)
			return entry;
	}

	return NULL;
}


const char *
	PllChipRegDescription(
	PLL_CHIP_TYPE Type,
	UCHAR Reg
	)
{
	PPLL_CHIP_DESC chip;

	chip = PllChipByType(Type);
	if (!chip)
		return NULL;

	if (Type <= PLL_UNKNOWN || Type >= (PLL_CHIP_TYPE)8)
		return NULL;

	if (Reg >= 64)
		return NULL;

	if (chip->RegDescriptions[Reg][0] == '\0')
		return NULL;

	return chip->RegDescriptions[Reg];
}


const char *
	PllChipRegDescriptionByKey(
	const char *Key,
	UCHAR Reg
	)
{
	PPLL_CHIP_DESC chip;

	chip = PllChipByKey(Key);
	if (!chip)
		return NULL;

	return PllChipRegDescription(chip->Type, Reg);
}


PPLL_CHIP_DESC
	PllChipMatch(
	UCHAR Address,
	UCHAR R0
	)
{
	PPLL_CHIP_DESC entry;
	int i;

	if (g_LoadedPllChips)
	{
		for (i = 0; i < g_LoadedPllCount; i++)
		{
			entry = &g_LoadedPllChips[i];

			if (entry->DefaultAddress != Address)
				continue;

			if (entry->SignatureMask != 0)
			{
				if ((R0 & entry->SignatureMask) != entry->SignatureValue)
					continue;
			}

			return entry;
		}
	}

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
	int i;

	if (g_LoadedPllChips)
	{
		for (i = 0; i < g_LoadedPllCount; i++)
		{
			if (g_LoadedPllChips[i].Type == Type)
				return &g_LoadedPllChips[i];
		}
	}

	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (entry->Type == Type)
			return entry;
	}

	return NULL;
}
