#pragma once

#include <Windows.h>


typedef enum
{
	PLL_UNKNOWN = 0,
	PLL_RTM880N_793,
	PLL_ICS_LPRS477,
	PLL_ICS_LPRS918,
	PLL_ICS_LPRS919

} PLL_CHIP_TYPE;


/* Describes a PLL clock generator chip model */
typedef struct _PLL_CHIP_DESC
{
	PLL_CHIP_TYPE   Type;
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
