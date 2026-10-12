#pragma once

#include <Windows.h>


typedef struct
{
	USHORT VendorId;
	USHORT DeviceId;
	char   ChipsetKey[32];

	const char *Name;

	/* PCI location and config offset of the SMBus I/O BAR */
	UCHAR SmbusSlotDev;
	UCHAR SmbusSlotFn;
	UCHAR SmbusBarOffset;
	ULONG SmbusBarMask;

	/* SMBHSTCNT Byte Data command: 0x48 on AMD (bits[3:2]), 0x42 on Intel (bits[2:0]) */
	UCHAR SmbCmdByteData;

	/* SMBus timing knobs for slower/quirky controllers */
	ULONG SmbPollTimeout;
	ULONG SmbYieldEvery;
	ULONG SmbRecoverPolls;

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
	ChipsetKeyFromString(
	    const char *Str,
	    char *KeyOut,
	    int KeyOutLen
	    );

const char *
	ChipsetKeyToString(
	    const CHIPSET_DB_ENTRY *Entry
	    );

BOOL
	ChipsetDbLookupByKey(
	    const char *Key,
	    CHIPSET_DB_ENTRY *Entry
	    );

void
	ChipsetDbFree(
	    void
	    );