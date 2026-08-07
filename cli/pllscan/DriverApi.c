#include <Windows.h>

#include "DriverApi.h"
#include "Ioctl.h"

BOOL
	DrvOpen(
	PHANDLE Driver
	)
{
	*Driver = CreateFile(
		L"\\\\.\\PLLDRV",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		);

	return (*Driver != INVALID_HANDLE_VALUE);
}


void
	DrvClose(
	HANDLE Driver
	)
{
	if (Driver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(Driver);
	}
}


BOOL
	DrvIoctl(
	HANDLE Driver,
	DWORD Ioctl,
	PVOID InBuffer,
	DWORD InLength,
	PVOID OutBuffer,
	DWORD OutLength,
	PDWORD BytesReturned
	)
{
	return DeviceIoControl(
		Driver,
		Ioctl,
		InBuffer,
		InLength,
		OutBuffer,
		OutLength,
		BytesReturned,
		NULL
		);
}

BOOL
	DrvPciReadConfig(
	HANDLE Driver,
	UCHAR Bus,
	UCHAR Device,
	UCHAR Function,
	UCHAR Offset,
	PULONG Value
	)
{
	PCI_READ_CONFIG_REQUEST req;

	DWORD bytes;


	req.Bus = Bus;
	req.Device = Device;
	req.Function = Function;
	req.Offset = Offset;


	if (!DrvIoctl(
		Driver,
		IOCTL_PLL_PCI_READ_CONFIG,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&bytes))
	{
		return FALSE;
	}


	*Value = req.Value;

	return TRUE;
}

BOOL
	DrvPciScan(
	HANDLE Driver,
	PVOID Buffer,
	ULONG BufferSize,
	PULONG BytesReturned
	)
{
	DWORD bytes;


	if (!DrvIoctl(
		Driver,
		IOCTL_PLL_SCAN_PCI,
		NULL,
		0,
		Buffer,
		BufferSize,
		&bytes))
	{
		return FALSE;
	}


	*BytesReturned = bytes;

	return TRUE;
}

BOOL
	DrvSmbusReadByte(
	HANDLE Driver,
	UCHAR Slave,
	UCHAR Command,
	PUCHAR Data,
	PUCHAR Status
	)
{
	SMBUS_READ_REQUEST req;

	DWORD bytes;


	req.SlaveAddress = Slave;
	req.Command = Command;
	req.Data = 0;
	req.Status = 0;


	if (!DrvIoctl(
		Driver,
		IOCTL_PLL_SMBUS_READ_BYTE,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&bytes))
	{
		return FALSE;
	}


	*Data = req.Data;
	*Status = req.Status;


	return TRUE;
}

BOOL
	DrvPortRead(
	HANDLE Driver,
	USHORT Port,
	PUCHAR Value
	)
{
	PORT_REQUEST req;
	DWORD bytes;


	req.Port = Port;
	req.Value = 0;


	if (!DrvIoctl(
		Driver,
		IOCTL_PLL_READ_PORT,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&bytes))
	{
		return FALSE;
	}


	*Value = req.Value;

	return TRUE;
}

BOOL
	DrvPortWrite(
	HANDLE Driver,
	USHORT Port,
	UCHAR Value
	)
{
	PORT_REQUEST req;
	DWORD bytes;


	req.Port = Port;
	req.Value = Value;


	return DrvIoctl(
		Driver,
		IOCTL_PLL_WRITE_PORT,
		&req,
		sizeof(req),
		&req,
		sizeof(req),
		&bytes);
}


BOOL
	DrvSmbusWriteByte(
	HANDLE Driver,
	UCHAR Slave,
	UCHAR Command,
	UCHAR Data
	)
{
	SMBUS_WRITE_REQUEST req;
	DWORD bytes;


	req.SlaveAddress = Slave;
	req.Command = Command;
	req.Data = Data;


	return DrvIoctl(
		Driver,
		IOCTL_PLL_SMBUS_WRITE_BYTE,
		&req,
		sizeof(req),
		NULL,
		0,
		&bytes);
}
