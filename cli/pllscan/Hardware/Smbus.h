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

BOOL
	SmbusQuickWrite(
	PPLL_CONTEXT Context,
	UCHAR Address
	);

/* Receive Byte: START+ADDR(R)+DATA+STOP — no register/command byte sent. */
BOOL
	SmbusReceiveByte(
	PPLL_CONTEXT Context,
	UCHAR Address,
	PUCHAR Data,
	PUCHAR Status
	);

/* Sequential register read using Receive Byte; resets pointer via Send Byte first. */
BOOL
	SmbusReadSeq(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR StartReg,
	UCHAR *Buf,
	UCHAR Count
	);