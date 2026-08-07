#pragma once

#include <Windows.h>

#define USER_MODE
#include "Ioctl.h"

BOOL
	DrvOpen(
	PHANDLE Driver
	);


void
	DrvClose(
	HANDLE Driver
	);


BOOL
	DrvIoctl(
	HANDLE Driver,
	DWORD Ioctl,
	PVOID InBuffer,
	DWORD InLength,
	PVOID OutBuffer,
	DWORD OutLength,
	PDWORD BytesReturned
	);


BOOL
	DrvPciReadConfig(
	HANDLE Driver,
	UCHAR Bus,
	UCHAR Device,
	UCHAR Function,
	UCHAR Offset,
	PULONG Value
	);


BOOL
	DrvPciScan(
	HANDLE Driver,
	PVOID Buffer,
	ULONG BufferSize,
	PULONG BytesReturned
	);


BOOL
	DrvSmbusReadByte(
	HANDLE Driver,
	UCHAR Slave,
	UCHAR Command,
	PUCHAR Data,
	PUCHAR Status
	);


BOOL
	DrvSmbusWriteByte(
	HANDLE Driver,
	UCHAR Slave,
	UCHAR Command,
	UCHAR Data
	);


BOOL
	DrvPortRead(
	HANDLE Driver,
	USHORT Port,
	PUCHAR Value
	);


BOOL
	DrvPortWrite(
	HANDLE Driver,
	USHORT Port,
	UCHAR Value
	);

