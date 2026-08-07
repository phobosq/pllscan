#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ChipsetDb.h"


static
	CHIPSET_DB_ENTRY
	g_Chipsets[] =
{
	/* terminator-only fallback; Chipsets.ini provides real entries */
	{ 0, 0, CHIPSET_UNKNOWN, NULL, 0, 0, 0, 0, 0 }
};


/* ------------------------------------------------------------------ */
/* Runtime table loaded from Chipsets.ini (NULL when not loaded)       */
/* ------------------------------------------------------------------ */

static CHIPSET_DB_ENTRY *g_LoadedChipsets;
static int               g_LoadedCount;


static
	CHIPSET_TYPE
	MapTypeString(
	    const char *Str
	    )
{
	char *end;
	unsigned long n;

	if (!Str || Str[0] == '\0')
		return CHIPSET_UNKNOWN;

	/* Allow explicit numeric type IDs in config for full decoupling. */
	n = strtoul(Str, &end, 0);
	if (*end == '\0')
	{
		if (n >= (unsigned long)CHIPSET_AMD_SB600 &&
		    n <= (unsigned long)CHIPSET_INTEL_PCH9)
		{
			return (CHIPSET_TYPE)n;
		}

		return CHIPSET_UNKNOWN;
	}

	if (_strnicmp(Str, "AMD_SB", 6) == 0)
	{
		n = strtoul(Str + 6, &end, 10);
		if (*end == '\0')
		{
			if (n == 600) return CHIPSET_AMD_SB600;
			if (n == 700) return CHIPSET_AMD_SB700;
			if (n == 710) return CHIPSET_AMD_SB710;
			if (n == 750) return CHIPSET_AMD_SB750;
			if (n == 800) return CHIPSET_AMD_SB800;
		}
	}

	if (_strnicmp(Str, "CHIPSET_AMD_SB", 14) == 0)
	{
		n = strtoul(Str + 14, &end, 10);
		if (*end == '\0')
		{
			if (n == 600) return CHIPSET_AMD_SB600;
			if (n == 700) return CHIPSET_AMD_SB700;
			if (n == 710) return CHIPSET_AMD_SB710;
			if (n == 750) return CHIPSET_AMD_SB750;
			if (n == 800) return CHIPSET_AMD_SB800;
		}
	}

	if (_strnicmp(Str, "INTEL_ICH", 9) == 0)
	{
		n = strtoul(Str + 9, &end, 10);
		if (*end == '\0')
		{
			if (n == 7)  return CHIPSET_INTEL_ICH7;
			if (n == 8)  return CHIPSET_INTEL_ICH8;
			if (n == 9)  return CHIPSET_INTEL_ICH9;
			if (n == 10) return CHIPSET_INTEL_ICH10;
		}
	}

	if (_strnicmp(Str, "CHIPSET_INTEL_ICH", 17) == 0)
	{
		n = strtoul(Str + 17, &end, 10);
		if (*end == '\0')
		{
			if (n == 7)  return CHIPSET_INTEL_ICH7;
			if (n == 8)  return CHIPSET_INTEL_ICH8;
			if (n == 9)  return CHIPSET_INTEL_ICH9;
			if (n == 10) return CHIPSET_INTEL_ICH10;
		}
	}

	if (_strnicmp(Str, "INTEL_PCH", 9) == 0)
	{
		n = strtoul(Str + 9, &end, 10);
		if (*end == '\0' && n >= 5 && n <= 9)
			return (CHIPSET_TYPE)(CHIPSET_INTEL_PCH5 + (n - 5));
	}

	if (_strnicmp(Str, "CHIPSET_INTEL_PCH", 17) == 0)
	{
		n = strtoul(Str + 17, &end, 10);
		if (*end == '\0' && n >= 5 && n <= 9)
			return (CHIPSET_TYPE)(CHIPSET_INTEL_PCH5 + (n - 5));
	}

	return CHIPSET_UNKNOWN;
}


BOOL
	ChipsetTypeFromString(
	    const char *Str,
	    CHIPSET_TYPE *TypeOut
	    )
{
	CHIPSET_TYPE type;

	if (!Str || !TypeOut || Str[0] == '\0')
		return FALSE;

	type = MapTypeString(Str);
	if (type == CHIPSET_UNKNOWN)
		return FALSE;

	*TypeOut = type;
	return TRUE;
}


const char *
	ChipsetTypeToString(
	    CHIPSET_TYPE Type
	    )
{
	switch (Type)
	{
	case CHIPSET_AMD_SB600:   return "AMD_SB600";
	case CHIPSET_AMD_SB700:   return "AMD_SB700";
	case CHIPSET_AMD_SB710:   return "AMD_SB710";
	case CHIPSET_AMD_SB750:   return "AMD_SB750";
	case CHIPSET_AMD_SB800:   return "AMD_SB800";
	case CHIPSET_INTEL_ICH7:  return "INTEL_ICH7";
	case CHIPSET_INTEL_ICH8:  return "INTEL_ICH8";
	case CHIPSET_INTEL_ICH9:  return "INTEL_ICH9";
	case CHIPSET_INTEL_ICH10: return "INTEL_ICH10";
	case CHIPSET_INTEL_PCH5:  return "INTEL_PCH5";
	case CHIPSET_INTEL_PCH6:  return "INTEL_PCH6";
	case CHIPSET_INTEL_PCH7:  return "INTEL_PCH7";
	case CHIPSET_INTEL_PCH8:  return "INTEL_PCH8";
	case CHIPSET_INTEL_PCH9:  return "INTEL_PCH9";
	default:                  return "UNKNOWN";
	}
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
		entries[i].Type     = MapTypeString(tmp);
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

		i++;
	}

	/* terminator */
	memset(&entries[i], 0, sizeof(entries[i]));

	ChipsetDbFree();

	g_LoadedChipsets = entries;
	g_LoadedCount    = i;

	return TRUE;
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