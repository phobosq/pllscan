#include "Ioctl.h"


NTSTATUS
	WritePort(
	PPORT_WRITE_REQUEST Request
	)
{
	WRITE_PORT_UCHAR(
		(PUCHAR)Request->Port,
		Request->Value
		);

	return STATUS_SUCCESS;
}

NTSTATUS
	ReadPort(
	PPORT_REQUEST Request
	)
{
	Request->Value =
		READ_PORT_UCHAR(
		(PUCHAR)Request->Port
		);

	return STATUS_SUCCESS;
}