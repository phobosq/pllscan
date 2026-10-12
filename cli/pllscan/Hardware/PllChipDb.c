#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "PllChipDb.h"


/* keep one dummy fallback entry; real data is loaded from PllChips.ini */
static PLL_CHIP_DESC g_PllChips[] =
{
	{ "", NULL, 0, 0, { 0 }, 0, { { 0 } }, 0, 0, 0, 0, 0, 0, PLL_READ_BYTE_DATA }
};

static PLL_CHIP_DESC *g_LoadedPllChips;
static int            g_LoadedPllCount;

static UCHAR g_ProbeAddresses[32];
static int   g_ProbeAddressCount;


static
	BOOL
	PllChipAddressMatches(
	const PLL_CHIP_DESC *Entry,
	UCHAR Address
	)
{
	UCHAR i;

	if (Entry->DefaultAddress == Address)
		return TRUE;

	for (i = 0; i < Entry->ProbeAddressCount; i++)
	{
		if (Entry->ProbeAddresses[i] == Address)
			return TRUE;
	}

	return FALSE;
}


static
	BOOL
	PllChipSignatureMatches(
	const PLL_CHIP_DESC *Entry,
	UCHAR R0
	)
{
	if (Entry->SignatureMask != 0)
	{
		if ((R0 & Entry->SignatureMask) != Entry->SignatureValue)
			return FALSE;
	}

	if (Entry->SignatureMask2 != 0)
	{
		if ((R0 & Entry->SignatureMask2) != Entry->SignatureValue2)
			return FALSE;
	}

	return TRUE;
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
		if (g_LoadedPllChips[i].ProbeAddressCount > 0)
		{
			for (j = 0; j < g_LoadedPllChips[i].ProbeAddressCount; j++)
			{
				addr = g_LoadedPllChips[i].ProbeAddresses[j];

				if (addr == 0)
					continue;

				exists = 0;
				{
					int k;
					for (k = 0; k < g_ProbeAddressCount; k++)
					{
						if (g_ProbeAddresses[k] == addr)
						{
							exists = 1;
							break;
						}
					}
				}

				if (!exists && g_ProbeAddressCount < (int)(sizeof(g_ProbeAddresses)))
					g_ProbeAddresses[g_ProbeAddressCount++] = addr;
			}
		}
		else
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
		char key[16];
		char probeCsv[128];
		char *ctx;
		char *tok;

		if (!PllChipKeyFromString(p, entries[i].PllKey, sizeof(entries[i].PllKey)))
			continue;

		GetPrivateProfileStringA(p, "Name", p, tmp, sizeof(tmp), Path);
		entries[i].Name = _strdup(tmp);

		GetPrivateProfileStringA(p, "DefaultAddress", "0", tmp, sizeof(tmp), Path);
		entries[i].DefaultAddress = (UCHAR)strtoul(tmp, NULL, 16);
		entries[i].ProbeAddressCount = 0;

		GetPrivateProfileStringA(p, "ProbeAddresses", "", probeCsv, sizeof(probeCsv), Path);
		if (probeCsv[0] != '\0')
		{
			tok = strtok_s(probeCsv, ",", &ctx);
			while (tok != NULL && entries[i].ProbeAddressCount < (UCHAR)(sizeof(entries[i].ProbeAddresses)))
			{
				unsigned long parsed;
				while (*tok == ' ' || *tok == '\t')
					tok++;

				parsed = strtoul(tok, NULL, 16);
				entries[i].ProbeAddresses[entries[i].ProbeAddressCount++] = (UCHAR)parsed;

				tok = strtok_s(NULL, ",", &ctx);
			}
		}

		GetPrivateProfileStringA(p, "RegisterCount", "0", tmp, sizeof(tmp), Path);
		entries[i].RegisterCount = (UCHAR)strtoul(tmp, NULL, 10);

		GetPrivateProfileStringA(p, "SignatureReg", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureReg = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureMask", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureMask = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureValue", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureValue = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureReg2", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureReg2 = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureMask2", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureMask2 = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SignatureValue2", "0", tmp, sizeof(tmp), Path);
		entries[i].SignatureValue2 = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "ReadProtocol", "ByteData", tmp, sizeof(tmp), Path);
		if (_stricmp(tmp, "Sequential") == 0 || strcmp(tmp, "1") == 0)
			entries[i].ReadProtocol = PLL_READ_SEQUENTIAL;
		else
			entries[i].ReadProtocol = PLL_READ_BYTE_DATA;
			
		for (r = 0; r < 64; r++)
		{
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
	PllChipRegDescriptionByKey(
	const char *Key,
	UCHAR Reg
	)
{
	PPLL_CHIP_DESC chip;

	chip = PllChipByKey(Key);
	if (!chip)
		return NULL;

	if (Reg >= 64)
		return NULL;

	if (chip->RegDescriptions[Reg][0] == '\0')
		return NULL;

	return chip->RegDescriptions[Reg];
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
		/* pass 1: require exact default address match */
		for (i = 0; i < g_LoadedPllCount; i++)
		{
			entry = &g_LoadedPllChips[i];

			if (entry->DefaultAddress != Address)
				continue;

			if (!PllChipSignatureMatches(entry, R0))
				continue;

			return entry;
		}

		/* pass 2: allow ProbeAddresses fallback */
		for (i = 0; i < g_LoadedPllCount; i++)
		{
			entry = &g_LoadedPllChips[i];

			if (!PllChipAddressMatches(entry, Address))
				continue;

			if (!PllChipSignatureMatches(entry, R0))
				continue;

			return entry;
		}
	}

	/* pass 1: built-in table exact default address */
	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (entry->DefaultAddress != Address)
			continue;

		if (!PllChipSignatureMatches(entry, R0))
			continue;

		return entry;
	}

	/* pass 2: built-in table probe-address fallback */
	for (entry = g_PllChips; entry->Name != NULL; entry++)
	{
		if (!PllChipAddressMatches(entry, Address))
			continue;

		if (!PllChipSignatureMatches(entry, R0))
			continue;

		return entry;
	}

	return NULL;
}


