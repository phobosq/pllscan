#pragma once

#include <Windows.h>


typedef enum
{
	CHIPSET_UNKNOWN = 0,

	CHIPSET_AMD_SB600,
	CHIPSET_AMD_SB700,
	CHIPSET_AMD_SB710,
	CHIPSET_AMD_SB750,
	CHIPSET_AMD_SB800,      /* Hudson: SB800/820/850/900 */

	CHIPSET_INTEL_ICH7,
	CHIPSET_INTEL_ICH8,
	CHIPSET_INTEL_ICH9,
	CHIPSET_INTEL_ICH10,
	CHIPSET_INTEL_PCH5,     /* Ibex Peak,    LGA1156 2009 */
	CHIPSET_INTEL_PCH6,     /* Cougar Point, LGA1155 2011 */
	CHIPSET_INTEL_PCH7,     /* Panther Point,LGA1155 2012 */
	CHIPSET_INTEL_PCH8,     /* Lynx Point,   LGA1150 2013 */
	CHIPSET_INTEL_PCH9      /* Wildcat Point,LGA1150 2014 */

} CHIPSET_TYPE;


typedef struct
{
	USHORT VendorId;
	USHORT DeviceId;

	CHIPSET_TYPE Type;

	const char *Name;

	/* PCI location and config offset of the SMBus I/O BAR */
	UCHAR SmbusSlotDev;
	UCHAR SmbusSlotFn;
	UCHAR SmbusBarOffset;
	ULONG SmbusBarMask;

	/* SMBHSTCNT Byte Data command: 0x48 on AMD (bits[3:2]), 0x42 on Intel (bits[2:0]) */
	UCHAR SmbCmdByteData;

} CHIPSET_DB_ENTRY;


BOOL
	ChipsetLookup(
	    USHORT VendorId,
	    USHORT DeviceId,
	    CHIPSET_DB_ENTRY *Entry
	    );

BOOL
	ChipsetDbLoad(
	    const char *Path
	    );

BOOL
	ChipsetTypeFromString(
	    const char *Str,
	    CHIPSET_TYPE *TypeOut
	    );

const char *
	ChipsetTypeToString(
	    CHIPSET_TYPE Type
	    );

void
	ChipsetDbFree(
	    void
	    );