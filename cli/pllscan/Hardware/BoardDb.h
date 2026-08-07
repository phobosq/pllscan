#pragma once

#include <Windows.h>
#include "PllChip.h"


typedef struct
{
	UCHAR Index;
	UCHAR Value;

} BOARD_REG_DEFAULT;


typedef struct
{
	char  Label[16];
	ULONG FreqMhz;

} BOARD_CLOCK_OUTPUT;


typedef struct
{
	char  Name[128];
	char  Chipset[32];        /* matches a CHIPSET_TYPE string, informational */
	char  Northbridge[64];

	PLL_CHIP_TYPE PllType;
	UCHAR         PllAddress;

	BOARD_REG_DEFAULT  Regs[64];
	UCHAR              RegCount;

	BOARD_CLOCK_OUTPUT Clocks[16];
	UCHAR              ClockCount;

} BOARD_PROFILE;


BOOL
	BoardDbLoad(
	    const char *Path
	    );

void
	BoardDbFree(
	    void
	    );

BOOL
	BoardDbLookup(
	    const char    *Name,
	    BOARD_PROFILE *Out
	    );

int
	BoardDbCount(
	    void
	    );

/* Populate profile fields into *Out for the named board; FALSE if not found */
BOOL
	BoardDbGetNames(
	    const char **NamesOut,
	    int          MaxNames,
	    int         *CountOut
	    );
