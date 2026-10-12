#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ChipsetDb.h"


static
	CHIPSET_DB_ENTRY
	g_Chipsets[] =
{
	/* terminator-only fallback; Chipsets.ini provides real entries */
		{ 0, 0, "", NULL, 0, 0, 0, 0, 0, 0, 0, 0 }
};


/* ------------------------------------------------------------------ */
/* Runtime table loaded from Chipsets.ini (NULL when not loaded)       */
/* ------------------------------------------------------------------ */

static CHIPSET_DB_ENTRY *g_LoadedChipsets;
static int               g_LoadedCount;

BOOL
	ChipsetKeyFromString(
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
	ChipsetKeyToString(
	    const CHIPSET_DB_ENTRY *Entry
	    )
{
	if (!Entry || Entry->ChipsetKey[0] == '\0')
		return "UNKNOWN";

	return Entry->ChipsetKey;
}


BOOL
	ChipsetDbLoad(
	    const char *Path
	    )
{
	char   sections[8192];
	char   tmp[256];
	char   name[128];
	const  char *p;
	int    count;
	int    i;
	CHIPSET_DB_ENTRY *entries;
	USHORT vid, did;

	if (GetPrivateProfileSectionNamesA(
		sections,
		sizeof(sections),
		Path) == 0)
	{
		return FALSE;
	}

	/* count non-empty sections */
	count = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
		count++;

	if (count == 0)
		return FALSE;

	entries = (CHIPSET_DB_ENTRY *)malloc(
		(count + 1) * sizeof(CHIPSET_DB_ENTRY));

	if (!entries)
		return FALSE;

	i = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
	{
		if (sscanf(p, "%hx:%hx", &vid, &did) != 2)
			continue;

		GetPrivateProfileStringA(p, "Type", "", tmp, sizeof(tmp), Path);
		GetPrivateProfileStringA(p, "Name", "", name, sizeof(name), Path);

		entries[i].VendorId = vid;
		entries[i].DeviceId = did;
		if (!ChipsetKeyFromString(tmp, entries[i].ChipsetKey, sizeof(entries[i].ChipsetKey)))
			entries[i].ChipsetKey[0] = '\0';
		entries[i].Name     = _strdup(name);

		GetPrivateProfileStringA(p, "SmbusSlotDev",   "0",    tmp, sizeof(tmp), Path);
		entries[i].SmbusSlotDev = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SmbusSlotFn",    "0",    tmp, sizeof(tmp), Path);
		entries[i].SmbusSlotFn  = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SmbusBarOffset", "0",    tmp, sizeof(tmp), Path);
		entries[i].SmbusBarOffset = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SmbusBarMask",   "FFE0", tmp, sizeof(tmp), Path);
		entries[i].SmbusBarMask = strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SmbCmdByteData", "48",   tmp, sizeof(tmp), Path);
		entries[i].SmbCmdByteData = (UCHAR)strtoul(tmp, NULL, 16);

		GetPrivateProfileStringA(p, "SmbPollTimeout", "10000", tmp, sizeof(tmp), Path);
		entries[i].SmbPollTimeout = strtoul(tmp, NULL, 10);

		GetPrivateProfileStringA(p, "SmbYieldEvery", "64", tmp, sizeof(tmp), Path);
		entries[i].SmbYieldEvery = strtoul(tmp, NULL, 10);

		GetPrivateProfileStringA(p, "SmbRecoverPolls", "1000", tmp, sizeof(tmp), Path);
		entries[i].SmbRecoverPolls = strtoul(tmp, NULL, 10);

		i++;
	}

	/* terminator */
	memset(&entries[i], 0, sizeof(entries[i]));

	ChipsetDbFree();

	g_LoadedChipsets = entries;
	g_LoadedCount    = i;

	return TRUE;
}


BOOL
	ChipsetDbLookupByKey(
	    const char *Key,
	    CHIPSET_DB_ENTRY *Entry
	    )
{
	int i;

	if (!Key || !Entry || Key[0] == '\0')
		return FALSE;

	if (g_LoadedChipsets)
	{
		for (i = 0; i < g_LoadedCount; i++)
		{
			if (_stricmp(g_LoadedChipsets[i].ChipsetKey, Key) == 0)
			{
				*Entry = g_LoadedChipsets[i];
				return TRUE;
			}
		}
	}

	for (i = 0; g_Chipsets[i].Name != NULL; i++)
	{
		if (_stricmp(g_Chipsets[i].ChipsetKey, Key) == 0)
		{
			*Entry = g_Chipsets[i];
			return TRUE;
		}
	}

	return FALSE;
}


void
	ChipsetDbFree(
	    void
	    )
{
	int i;

	if (!g_LoadedChipsets)
		return;

	for (i = 0; i < g_LoadedCount; i++)
		free((void *)g_LoadedChipsets[i].Name);

	free(g_LoadedChipsets);

	g_LoadedChipsets = NULL;
	g_LoadedCount    = 0;
}


BOOL
	ChipsetLookup(
	    USHORT VendorId,
	    USHORT DeviceId,
	    CHIPSET_DB_ENTRY *Entry
	    )
{
	int i;

	/* prefer the runtime table when loaded */
	if (g_LoadedChipsets)
	{
		for (i = 0; i < g_LoadedCount; i++)
		{
			if (g_LoadedChipsets[i].VendorId == VendorId &&
			    g_LoadedChipsets[i].DeviceId == DeviceId)
			{
				*Entry = g_LoadedChipsets[i];
				return TRUE;
			}
		}
	}

	for (i = 0; g_Chipsets[i].Name != NULL; i++)
	{
		if (g_Chipsets[i].VendorId == VendorId &&
		    g_Chipsets[i].DeviceId == DeviceId)
		{
			*Entry = g_Chipsets[i];
			return TRUE;
		}
	}

	return FALSE;
}