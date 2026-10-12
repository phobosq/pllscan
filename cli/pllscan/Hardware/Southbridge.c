#include <stdio.h>

#include "Southbridge.h"
#include "..\DriverApi.h"


BOOL
	SouthbridgeDetect(
	PPLL_CONTEXT Context
	)
{
	ULONG value;

	USHORT vendor;
	USHORT device;

	CHIPSET_DB_ENTRY entry;

	static const struct { UCHAR Dev; UCHAR Fn; } slots[] =
	{
		{ 0x14, 0 }, /* AMD SB */
		{ 0x1F, 0 }  /* Intel ICH LPC */
	};

	int i;

	for (i = 0; i < 2; i++)
	{
		if (!DrvPciReadConfig(
			Context->Driver,
			0,
			slots[i].Dev,
			slots[i].Fn,
			0,
			&value))
		{
			continue;
		}

		vendor = (USHORT)(value & 0xFFFF);
		device = (USHORT)((value >> 16) & 0xFFFF);

		if (vendor == 0xFFFF)
			continue;

		if (!ChipsetLookup(vendor, device, &entry))
			continue;

		Context->Platform.SouthbridgeVendor = vendor;
		Context->Platform.SouthbridgeDevice = device;
		strncpy(
			Context->Platform.SouthbridgeKey,
			entry.ChipsetKey,
			sizeof(Context->Platform.SouthbridgeKey) - 1);
		Context->Platform.SouthbridgeKey[sizeof(Context->Platform.SouthbridgeKey) - 1] = '\0';

		strcpy_s(
			Context->Platform.SouthbridgeName,
			sizeof(Context->Platform.SouthbridgeName),
			entry.Name);

		return TRUE;
	}

	return FALSE;
}

BOOL
	SouthbridgeReadResources(
	PPLL_CONTEXT Context
	)
{
	ULONG value;
	CHIPSET_DB_ENTRY entry;

	if (!ChipsetLookup(
		Context->Platform.SouthbridgeVendor,
		Context->Platform.SouthbridgeDevice,
		&entry))
	{
		return FALSE;
	}

	if (!DrvPciReadConfig(
		Context->Driver,
		0,
		entry.SmbusSlotDev,
		entry.SmbusSlotFn,
		entry.SmbusBarOffset,
		&value))
	{
		return FALSE;
	}

	Context->Platform.SmbusBase = (USHORT)(value & entry.SmbusBarMask);
	Context->Platform.SmbCmdByteData = entry.SmbCmdByteData;
	Context->Platform.SmbPollTimeout = entry.SmbPollTimeout;
	Context->Platform.SmbYieldEvery = entry.SmbYieldEvery;
	Context->Platform.SmbRecoverPolls = entry.SmbRecoverPolls;

	return TRUE;
}