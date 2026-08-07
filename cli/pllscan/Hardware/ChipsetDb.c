#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ChipsetDb.h"


/* VendorId DeviceId Type Name SmbusSlotDev SmbusSlotFn SmbusBarOffset SmbusBarMask */
#define AMD_SB(did, type, name)  { 0x1002, did, type, name, 0x14, 0, 0x90, 0xFFE0, 0x48 }
#define ICH(did, type, name)     { 0x8086, did, type, name, 0x1F, 3, 0x20, 0xFFE0, 0x42 }

static
	CHIPSET_DB_ENTRY
	g_Chipsets[] =
{
	/* AMD SB-series (SMBus BAR at PCI 00:14.0) */
	{ 0x1002, 0x4372, CHIPSET_AMD_SB600, "AMD SB600",         0x14, 0, 0x14, 0xFFE0, 0x48 }, /* verify offset */
	AMD_SB(0x4385, CHIPSET_AMD_SB750, "AMD SB750"),
	AMD_SB(0x4390, CHIPSET_AMD_SB700, "AMD SB700"),
	AMD_SB(0x4391, CHIPSET_AMD_SB700, "AMD SB710"),
	{ 0x1002, 0x780B, CHIPSET_AMD_SB800, "AMD SB800/850/900", 0x14, 0, 0x10, 0xFFE0, 0x48 },

	/* Intel ICH7 (LPC bridge at PCI 00:1F.0, SMBus BAR at 00:1F.3 offset 0x20) */
	ICH(0x27B8, CHIPSET_INTEL_ICH7, "Intel ICH7"  ),
	ICH(0x27B9, CHIPSET_INTEL_ICH7, "Intel ICH7-M"),
	ICH(0x27BC, CHIPSET_INTEL_ICH7, "Intel ICH7-V"),
	ICH(0x27BD, CHIPSET_INTEL_ICH7, "Intel ICH7-M DH"),

	/* Intel ICH8 */
	ICH(0x2810, CHIPSET_INTEL_ICH8, "Intel ICH8"  ),
	ICH(0x2812, CHIPSET_INTEL_ICH8, "Intel ICH8DH"),
	ICH(0x2814, CHIPSET_INTEL_ICH8, "Intel ICH8DO"),
	ICH(0x2815, CHIPSET_INTEL_ICH8, "Intel ICH8M" ),

	/* Intel ICH9 */
	ICH(0x2910, CHIPSET_INTEL_ICH9, "Intel ICH9"   ),
	ICH(0x2912, CHIPSET_INTEL_ICH9, "Intel ICH9DH" ),
	ICH(0x2914, CHIPSET_INTEL_ICH9, "Intel ICH9DO" ),
	ICH(0x2916, CHIPSET_INTEL_ICH9, "Intel ICH9R"  ),
	ICH(0x2918, CHIPSET_INTEL_ICH9, "Intel ICH9M"  ),
	ICH(0x2919, CHIPSET_INTEL_ICH9, "Intel ICH9ME" ),

	/* Intel ICH10 */
	ICH(0x3A14, CHIPSET_INTEL_ICH10, "Intel ICH10D"),
	ICH(0x3A16, CHIPSET_INTEL_ICH10, "Intel ICH10R"),
	ICH(0x3A18, CHIPSET_INTEL_ICH10, "Intel ICH10" ),
	ICH(0x3A1A, CHIPSET_INTEL_ICH10, "Intel ICH10D"),
	ICH(0x3A30, CHIPSET_INTEL_ICH10, "Intel ICH10" ),

	/* Intel 5-series PCH (Ibex Peak, LGA1156 2009) */
	ICH(0x3B00, CHIPSET_INTEL_PCH5, "Intel PCH P55"  ),
	ICH(0x3B02, CHIPSET_INTEL_PCH5, "Intel PCH H55"  ),
	ICH(0x3B04, CHIPSET_INTEL_PCH5, "Intel PCH PM55" ),
	ICH(0x3B06, CHIPSET_INTEL_PCH5, "Intel PCH H57"  ),
	ICH(0x3B08, CHIPSET_INTEL_PCH5, "Intel PCH Q57"  ),
	ICH(0x3B09, CHIPSET_INTEL_PCH5, "Intel PCH HM55" ),
	ICH(0x3B0B, CHIPSET_INTEL_PCH5, "Intel PCH HM57" ),
	ICH(0x3B12, CHIPSET_INTEL_PCH5, "Intel PCH QM57" ),
	ICH(0x3B14, CHIPSET_INTEL_PCH5, "Intel PCH QS57" ),

	/* Intel 6-series PCH (Cougar Point, LGA1155 2011) */
	ICH(0x1C44, CHIPSET_INTEL_PCH6, "Intel PCH Z68"  ),
	ICH(0x1C46, CHIPSET_INTEL_PCH6, "Intel PCH P67"  ),
	ICH(0x1C49, CHIPSET_INTEL_PCH6, "Intel PCH HM65" ),
	ICH(0x1C4A, CHIPSET_INTEL_PCH6, "Intel PCH H67"  ),
	ICH(0x1C4B, CHIPSET_INTEL_PCH6, "Intel PCH HM67" ),
	ICH(0x1C4E, CHIPSET_INTEL_PCH6, "Intel PCH Q67"  ),
	ICH(0x1C4F, CHIPSET_INTEL_PCH6, "Intel PCH QM67" ),
	ICH(0x1C50, CHIPSET_INTEL_PCH6, "Intel PCH B65"  ),
	ICH(0x1C52, CHIPSET_INTEL_PCH6, "Intel PCH Q65"  ),
	ICH(0x1C53, CHIPSET_INTEL_PCH6, "Intel PCH QS67" ),
	ICH(0x1C5C, CHIPSET_INTEL_PCH6, "Intel PCH H61"  ),

	/* Intel 7-series PCH (Panther Point, LGA1155 2012) */
	ICH(0x1E44, CHIPSET_INTEL_PCH7, "Intel PCH Z77"  ),
	ICH(0x1E46, CHIPSET_INTEL_PCH7, "Intel PCH Z75"  ),
	ICH(0x1E4A, CHIPSET_INTEL_PCH7, "Intel PCH H77"  ),
	ICH(0x1E55, CHIPSET_INTEL_PCH7, "Intel PCH Q77"  ),
	ICH(0x1E58, CHIPSET_INTEL_PCH7, "Intel PCH B75"  ),
	ICH(0x1E5C, CHIPSET_INTEL_PCH7, "Intel PCH H61"  ),
	ICH(0x1E57, CHIPSET_INTEL_PCH7, "Intel PCH HM77" ),
	ICH(0x1E59, CHIPSET_INTEL_PCH7, "Intel PCH HM75" ),
	ICH(0x1E5E, CHIPSET_INTEL_PCH7, "Intel PCH QM77" ),

	/* Intel 8-series PCH (Lynx Point, LGA1150 2013) */
	ICH(0x8C44, CHIPSET_INTEL_PCH8, "Intel PCH Z87"  ),
	ICH(0x8C46, CHIPSET_INTEL_PCH8, "Intel PCH Z85"  ),
	ICH(0x8C49, CHIPSET_INTEL_PCH8, "Intel PCH HM86" ),
	ICH(0x8C4A, CHIPSET_INTEL_PCH8, "Intel PCH H87"  ),
	ICH(0x8C4B, CHIPSET_INTEL_PCH8, "Intel PCH HM87" ),
	ICH(0x8C4F, CHIPSET_INTEL_PCH8, "Intel PCH QM87" ),
	ICH(0x8C50, CHIPSET_INTEL_PCH8, "Intel PCH B85"  ),
	ICH(0x8C54, CHIPSET_INTEL_PCH8, "Intel PCH Q87"  ),
	ICH(0x8C56, CHIPSET_INTEL_PCH8, "Intel PCH Q85"  ),
	ICH(0x8C5C, CHIPSET_INTEL_PCH8, "Intel PCH H81"  ),

	/* Intel 9-series PCH (Wildcat Point, LGA1150 2014) */
	ICH(0x8CC3, CHIPSET_INTEL_PCH9, "Intel PCH HM97" ),
	ICH(0x8CC4, CHIPSET_INTEL_PCH9, "Intel PCH Z97"  ),
	ICH(0x8CC6, CHIPSET_INTEL_PCH9, "Intel PCH H97"  ),
	ICH(0x8CCA, CHIPSET_INTEL_PCH9, "Intel PCH QM87" ),

	/* terminator */
	{ 0, 0, CHIPSET_UNKNOWN, NULL, 0, 0, 0, 0, 0 }
};

#undef AMD_SB
#undef ICH


/* ------------------------------------------------------------------ */
/* Runtime table loaded from Chipsets.ini (NULL when not loaded)       */
/* ------------------------------------------------------------------ */

static CHIPSET_DB_ENTRY *g_LoadedChipsets;
static int               g_LoadedCount;


static const struct { const char *Str; CHIPSET_TYPE Type; } g_TypeMap[] =
{
	{ "AMD_SB600",   CHIPSET_AMD_SB600  },
	{ "AMD_SB700",   CHIPSET_AMD_SB700  },
	{ "AMD_SB710",   CHIPSET_AMD_SB710  },
	{ "AMD_SB750",   CHIPSET_AMD_SB750  },
	{ "AMD_SB800",   CHIPSET_AMD_SB800  },
	{ "INTEL_ICH7",  CHIPSET_INTEL_ICH7 },
	{ "INTEL_ICH8",  CHIPSET_INTEL_ICH8 },
	{ "INTEL_ICH9",  CHIPSET_INTEL_ICH9 },
	{ "INTEL_ICH10", CHIPSET_INTEL_ICH10},
	{ "INTEL_PCH5",  CHIPSET_INTEL_PCH5 },
	{ "INTEL_PCH6",  CHIPSET_INTEL_PCH6 },
	{ "INTEL_PCH7",  CHIPSET_INTEL_PCH7 },
	{ "INTEL_PCH8",  CHIPSET_INTEL_PCH8 },
	{ "INTEL_PCH9",  CHIPSET_INTEL_PCH9 },
	{ NULL,          CHIPSET_UNKNOWN    }
};


static
	CHIPSET_TYPE
	MapTypeString(
	    const char *Str
	    )
{
	int i;

	for (i = 0; g_TypeMap[i].Str != NULL; i++)
	{
		if (_stricmp(Str, g_TypeMap[i].Str) == 0)
			return g_TypeMap[i].Type;
	}

	return CHIPSET_UNKNOWN;
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