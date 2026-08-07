#include "Platform.h"
#include "Southbridge.h"

BOOL
	PlatformDetect(
	PPLL_CONTEXT Context
	)
{
	ZeroMemory(
		&Context->Platform,
		sizeof(Context->Platform)
		);


	if (!SouthbridgeDetect(Context))
	{
		return FALSE;
	}


	if (!SouthbridgeReadResources(Context))
	{
		return FALSE;
	}


	return TRUE;
}