#pragma once

#include <Windows.h>
#include "ChipsetDb.h"
#include "PllChip.h"
#include "BoardDb.h"


typedef struct
{
	CHIPSET_TYPE Chipset;

	USHORT VendorId;
	USHORT DeviceId;

	USHORT SmbusBase;
	USHORT SouthbridgeVendor;
	USHORT SouthbridgeDevice;

	CHIPSET_TYPE SouthbridgeType;

	char SouthbridgeName[64];

	PLL_CHIP_TYPE PllType;
	UCHAR         PllAddress;
	char          PllName[48];

	char BoardName[128];
	char NorthbridgeName[64];

	/* chipset-specific SMBHSTCNT value for Byte Data transactions */
	UCHAR SmbCmdByteData;

} PLATFORM_INFO;


typedef struct _PLL_CONTEXT
{
	HANDLE Driver;
	PLATFORM_INFO Platform;
} PLL_CONTEXT, *PPLL_CONTEXT;