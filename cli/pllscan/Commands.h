#pragma once

#include <Windows.h>
#include "Context.h"

int
	CmdPci(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);

int
	CmdPort(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);

int
	CmdSmbus(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);

int
	CmdPlatform(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);

int
	CmdPll(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);

int
	CmdBoard(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
);