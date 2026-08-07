#pragma once
#include <Windows.h>

/* Accepts decimal or 0x-prefixed hex */
ULONG
	ParseInt(
	    const char *Text
		);

ULONG
	ParseHex(
	    const char *Text
		);

void
	PrintUsage(
	    void
		);