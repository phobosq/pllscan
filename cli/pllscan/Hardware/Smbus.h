#pragma once

#include <Windows.h>

#include "Context.h"

USHORT
	SmbusRegister(
	PPLL_CONTEXT Context,
	USHORT Offset
	);

BOOL
	SmbusReadByte(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR Command,
	PUCHAR Data,
	PUCHAR Status
	);

BOOL
	SmbusWriteByte(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR Command,
	UCHAR Value
	);

BOOL
	SmbusQuickRead(
	PPLL_CONTEXT Context,
	UCHAR Address
	);