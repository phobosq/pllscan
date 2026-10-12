#pragma once

#include <Windows.h>

/* INI key ReadProtocol values: "ByteData" (default) or "Sequential" */
typedef enum _PLL_READ_PROTOCOL
{
    PLL_READ_BYTE_DATA  = 0,
    PLL_READ_SEQUENTIAL = 1
} PLL_READ_PROTOCOL;

/* Describes a PLL clock generator chip model */
typedef struct _PLL_CHIP_DESC
{
	char              PllKey[32];
    const char       *Name;
    UCHAR             DefaultAddress;
	UCHAR             ProbeAddressCount;
	UCHAR             ProbeAddresses[8];
    UCHAR             RegisterCount;
    char              RegDescriptions[64][64];
    UCHAR             SignatureReg;
    UCHAR             SignatureMask;
    UCHAR             SignatureValue;
	UCHAR             SignatureReg2;
	UCHAR             SignatureMask2;
	UCHAR             SignatureValue2;
    PLL_READ_PROTOCOL ReadProtocol;
} PLL_CHIP_DESC, *PPLL_CHIP_DESC;
