#pragma once

#include <Windows.h>

/* Describes a PLL clock generator chip model */
typedef struct _PLL_CHIP_DESC
{
	char            PllKey[32];
	const char     *Name;
	UCHAR           DefaultAddress;
	UCHAR           RegisterCount;
	char            RegDescriptions[64][64];

	/*
	R0 signature match: (ReadByte(SignatureReg) & SignatureMask) == SignatureValue
	Set SignatureMask = 0 when signature is not yet verified for this chip.
	*/
	UCHAR           SignatureReg;
	UCHAR           SignatureMask;
	UCHAR           SignatureValue;

} PLL_CHIP_DESC, *PPLL_CHIP_DESC;
