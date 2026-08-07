#include "Ioctl.h"
#include "plldrv.h"

#define SMBUS_BASE 0x0B00

#define SMBHSTSTS   0x00
#define SMBHSTCNT   0x02
#define SMBHSTCMD   0x03
#define SMBHSTADD   0x04
#define SMBHSTDAT0  0x05


NTSTATUS
	SmbusReadByte(
	PSMBUS_READ_REQUEST Request
	)
{
	UCHAR status;
	ULONG timeout;
	LARGE_INTEGER interval;
	NTSTATUS result;
	NTSTATUS waitStatus;
	BOOLEAN mutexHeld;

	result = STATUS_SUCCESS;
	mutexHeld = FALSE;

	waitStatus = KeWaitForSingleObject(
		&g_SmbusMutex,
		Executive,
		KernelMode,
		FALSE,
		NULL
		);

	if (!NT_SUCCESS(waitStatus))
	{
		return waitStatus;
	}

	mutexHeld = TRUE;


	/*
	Clear previous status
	*/

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTSTS),
		0xFF
		);


	/*
	Set slave address
	bit0 = read
	*/

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTADD),
		(Request->SlaveAddress << 1) | 1
		);


	/*
	Command/register
	*/

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTCMD),
		Request->Command
		);


	/*
	Start byte read
	(SMBus host control)
	*/

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTCNT),
		0x48
		);


	/*
	Wait for completion
	*/

	timeout = 10000;

	do
	{
		status = READ_PORT_UCHAR(
			(PUCHAR)(SMBUS_BASE + SMBHSTSTS)
			);

		if (status & 0x02)
		{
			break;
		}

		if (timeout > 5000)
		{
			KeStallExecutionProcessor(10);
		}
		else
		{
			interval.QuadPart = -10000;
			KeDelayExecutionThread(
				KernelMode,
				FALSE,
				&interval
				);
		}

	} while (--timeout);


	Request->Status = status;


	if (timeout == 0)
	{
		result = STATUS_TIMEOUT;
		goto Exit;
	}


	/*
	Read data
	*/

	Request->Data =
		READ_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTDAT0)
		);

	result = STATUS_SUCCESS;

Exit:
	if (mutexHeld)
	{
		KeReleaseMutex(&g_SmbusMutex, FALSE);
	}

	return result;
}


NTSTATUS
	SmbusWriteByte(
	PSMBUS_WRITE_REQUEST Request
	)
{
	UCHAR status;
	ULONG timeout;
	LARGE_INTEGER interval;
	NTSTATUS result;
	NTSTATUS waitStatus;
	BOOLEAN mutexHeld;

	result = STATUS_SUCCESS;
	mutexHeld = FALSE;

	waitStatus = KeWaitForSingleObject(
		&g_SmbusMutex,
		Executive,
		KernelMode,
		FALSE,
		NULL
		);

	if (!NT_SUCCESS(waitStatus))
	{
		return waitStatus;
	}

	mutexHeld = TRUE;

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTSTS),
		0xFF
		);

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTADD),
		(Request->SlaveAddress << 1) & ~1
		);

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTCMD),
		Request->Command
		);

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTDAT0),
		Request->Data
		);

	WRITE_PORT_UCHAR(
		(PUCHAR)(SMBUS_BASE + SMBHSTCNT),
		0x48
		);

	timeout = 10000;

	do
	{
		status = READ_PORT_UCHAR(
			(PUCHAR)(SMBUS_BASE + SMBHSTSTS)
			);

		if (status & 0x02)
		{
			break;
		}

		if (status & 0xF8)
		{
			result = STATUS_DEVICE_DATA_ERROR;
			goto Exit;
		}

		if (timeout > 5000)
		{
			KeStallExecutionProcessor(10);
		}
		else
		{
			interval.QuadPart = -10000;
			KeDelayExecutionThread(
				KernelMode,
				FALSE,
				&interval
				);
		}

	} while (--timeout);

	if (timeout == 0)
	{
		result = STATUS_TIMEOUT;
	}

Exit:
	if (mutexHeld)
	{
		KeReleaseMutex(&g_SmbusMutex, FALSE);
	}

	return result;
}