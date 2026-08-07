#include "DriverApi.h"

#include "Smbus.h"
#include "SmbusRegs.h"


USHORT
	SmbusRegister(
	PPLL_CONTEXT Context,
	USHORT Offset
	)
{
	return (USHORT)(Context->Platform.SmbusBase + Offset);
}


static
	BOOL
	SmbusReadPort(
	PPLL_CONTEXT Context,
	USHORT Offset,
	PUCHAR Value
	)
{
	return DrvPortRead(
		Context->Driver,
		SmbusRegister(Context, Offset),
		Value
		);
}

static
	BOOL
	SmbusWritePort(
	PPLL_CONTEXT Context,
	USHORT Offset,
	UCHAR Value
	)
{
	return DrvPortWrite(
		Context->Driver,
		SmbusRegister(Context, Offset),
		Value
		);
}

/* Clears status; if HOST_BUSY is stuck, issues KILL to generate a STOP */
static
	BOOL
	SmbusWaitReady(
	PPLL_CONTEXT Context
	)
{
	UCHAR s;
	ULONG i;

	if (!SmbusWritePort(Context, SMBHSTSTS, 0xFF))
		return FALSE;

	if (!SmbusReadPort(Context, SMBHSTSTS, &s))
		return FALSE;

	if (s & SMB_STS_HOST_BUSY)
	{
		/* bit 4 = KILL: forces a STOP condition and releases HOST_BUSY */
		SmbusWritePort(Context, SMBHSTCNT, 0x10);

		for (i = 0; i < 1000; i++)
		{
			if (!SmbusReadPort(Context, SMBHSTSTS, &s))
				return FALSE;

			if (!(s & SMB_STS_HOST_BUSY))
				break;
		}

		SmbusWritePort(Context, SMBHSTSTS, 0xFF);

		if (s & SMB_STS_HOST_BUSY)
			return FALSE;
	}

	return TRUE;
}

BOOL
	SmbusReadByte(
	PPLL_CONTEXT Context,
	UCHAR Slave,
	UCHAR Command,
	PUCHAR Data,
	PUCHAR Status
	)
{
	UCHAR hostStatus;
	ULONG timeout;

	if (!SmbusWaitReady(Context))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTADD, (Slave << 1) | 1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCMD, Command))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, Context->Platform.SmbCmdByteData))
		return FALSE;

	timeout = 10000;
	do
	{
		if (!SmbusReadPort(Context, SMBHSTSTS, &hostStatus))
			return FALSE;

		if (hostStatus & SMB_ERR_MASK)
		{
			*Status = hostStatus;
			return FALSE;
		}

		if (hostStatus & SMB_STS_INTR)
			break;

	} while (--timeout);

	if (timeout == 0)
		return FALSE;

	*Status = hostStatus;

	if (!SmbusReadPort(Context, SMBHSTDAT0, Data))
		return FALSE;

	return TRUE;
}


BOOL
	SmbusWriteByte(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR Command,
	UCHAR Value
	)
{
	UCHAR hostStatus;
	ULONG timeout;

	if (!SmbusWaitReady(Context))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) & ~1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCMD, Command))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTDAT0, Value))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, Context->Platform.SmbCmdByteData))
		return FALSE;

	timeout = 10000;
	do
	{
		if (!SmbusReadPort(Context, SMBHSTSTS, &hostStatus))
			return FALSE;

		if (hostStatus & SMB_ERR_MASK)
			return FALSE;

		if (hostStatus & SMB_STS_INTR)
			return TRUE;

	} while (--timeout);

	return FALSE;
}


BOOL
	SmbusQuickRead(
	PPLL_CONTEXT Context,
	UCHAR Address
	)
{
	UCHAR hostStatus;
	ULONG timeout;

	if (!SmbusWaitReady(Context))
		return FALSE;

	/* bit 0 = 1: read direction */
	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) | 1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, SMB_CMD_QUICK))
		return FALSE;

	timeout = 10000;
	do
	{
		if (!SmbusReadPort(Context, SMBHSTSTS, &hostStatus))
			return FALSE;

		if (hostStatus & SMB_STS_DEV_ERR)
			return FALSE;

		if (hostStatus & SMB_STS_INTR)
			return TRUE;

	} while (--timeout);

	return FALSE;
}
