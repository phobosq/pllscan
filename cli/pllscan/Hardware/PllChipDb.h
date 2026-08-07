#pragma once

#include "PllChip.h"
#include "Context.h"


/*
Returns the first database entry whose address and signature match,
or NULL if no entry matches.
Address match is always required; signature match is skipped when
SignatureMask == 0 (unverified chips).
*/
PPLL_CHIP_DESC
	PllChipMatch(
	UCHAR Address,
	UCHAR R0
	);


/*
Returns the database entry for a given chip type, or NULL.
*/
PPLL_CHIP_DESC
	PllChipByType(
	PLL_CHIP_TYPE Type
	);
