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

static
	BOOL
	SmbusPollTransaction(
	PPLL_CONTEXT Context,
	PUCHAR Status
	)
{
	UCHAR hostStatus = 0;
	ULONG timeout;
	ULONG yieldEvery;

	timeout = Context->Platform.SmbPollTimeout;
	if (timeout == 0)
		timeout = 10000;

	yieldEvery = Context->Platform.SmbYieldEvery;
	if (yieldEvery == 0)
		yieldEvery = 64;

	do
	{
		if (!SmbusReadPort(Context, SMBHSTSTS, &hostStatus))
			return FALSE;

		if (hostStatus & SMB_ERR_MASK)
		{
			if (Status)
				*Status = hostStatus;
			return FALSE;
		}

		if (hostStatus & SMB_STS_INTR)
		{
			if (Status)
				*Status = hostStatus;
			return TRUE;
		}

		if ((timeout % yieldEvery) == 0)
			Sleep(0);

	} while (--timeout);

	if (Status)
		*Status = hostStatus;

	return FALSE;
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
	ULONG recoverPolls;
	ULONG yieldEvery;

	recoverPolls = Context->Platform.SmbRecoverPolls;
	if (recoverPolls == 0)
		recoverPolls = 1000;

	yieldEvery = Context->Platform.SmbYieldEvery;
	if (yieldEvery == 0)
		yieldEvery = 64;

	if (!SmbusWritePort(Context, SMBHSTSTS, 0xFF))
		return FALSE;

	if (!SmbusReadPort(Context, SMBHSTSTS, &s))
		return FALSE;

	if (s & SMB_STS_HOST_BUSY)
	{
		if (!SmbusWritePort(Context, SMBHSTCNT, 0x02))
			return FALSE;

		for (i = 0; i < recoverPolls; i++)
		{
			if (!SmbusReadPort(Context, SMBHSTSTS, &s))
				return FALSE;

			if (!(s & SMB_STS_HOST_BUSY))
				break;

			if ((i % yieldEvery) == 0)
				Sleep(0);
		}

		if (!SmbusWritePort(Context, SMBHSTSTS, 0xFF))
			return FALSE;

		if (!SmbusReadPort(Context, SMBHSTSTS, &s))
			return FALSE;

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

	if (Status)
		*Status = 0xFF;

	if (!SmbusWaitReady(Context))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTADD, (Slave << 1) | 1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCMD, Command))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, Context->Platform.SmbCmdByteData))
		return FALSE;

	if (!SmbusPollTransaction(Context, &hostStatus))
	{
		if (Status)
			*Status = hostStatus;
		return FALSE;
}

	if (Status)
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

	if (!SmbusPollTransaction(Context, &hostStatus))
		return FALSE;

	return TRUE;
}


BOOL
	SmbusQuickRead(
	PPLL_CONTEXT Context,
	UCHAR Address
	)
{
	UCHAR hostStatus;

	if (!SmbusWaitReady(Context))
		return FALSE;

	/* bit 0 = 1: read direction */
	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) | 1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, SMB_CMD_QUICK))
		return FALSE;

	if (!SmbusPollTransaction(Context, &hostStatus))
		return FALSE;

	return TRUE;
}


BOOL
	SmbusQuickWrite(
	PPLL_CONTEXT Context,
	UCHAR Address
	)
{
	UCHAR hostStatus;

	if (!SmbusWaitReady(Context))
		return FALSE;

	/* bit 0 = 0: write direction */
	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) & ~1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, SMB_CMD_QUICK))
		return FALSE;

	if (!SmbusPollTransaction(Context, &hostStatus))
		return FALSE;

	return TRUE;
}


BOOL
	SmbusReceiveByte(
	PPLL_CONTEXT Context,
	UCHAR Address,
	PUCHAR Data,
	PUCHAR Status
	)
{
	UCHAR hostStatus;

	if (Status)
		*Status = 0xFF;

	if (!SmbusWaitReady(Context))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) | 1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, SMB_CMD_BYTE))
		return FALSE;

	if (!SmbusPollTransaction(Context, &hostStatus))
	{
		if (Status)
			*Status = hostStatus;
		return FALSE;
}

	if (Status)
		*Status = hostStatus;

	if (!SmbusReadPort(Context, SMBHSTDAT0, Data))
		return FALSE;

	return TRUE;
}


BOOL
	SmbusReadSeq(
	PPLL_CONTEXT Context,
	UCHAR Address,
	UCHAR StartReg,
	UCHAR *Buf,
	UCHAR Count
	)
{
	UCHAR i;
	UCHAR status;

	/* set register pointer via Send Byte (write-direction, no data payload) */
	if (!SmbusWaitReady(Context))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTADD, (Address << 1) & ~1))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCMD, StartReg))
		return FALSE;

	if (!SmbusWritePort(Context, SMBHSTCNT, SMB_CMD_BYTE))
		return FALSE;

	{
		UCHAR hostStatus;
		if (!SmbusPollTransaction(Context, &hostStatus))
			return FALSE;
	}

	/* sequential receive: chip auto-increments register pointer */
	for (i = 0; i < Count; i++)
	{
		if (!SmbusReceiveByte(Context, Address, &Buf[i], &status))
			return FALSE;
	}

	return TRUE;
}
