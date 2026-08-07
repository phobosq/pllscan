#include <stdio.h>

#include "DriverApi.h"
#include "Commands.h"
#include "Util.h"
#include "Context.h"
#include "Platform.h"
#include "Hardware/BoardDb.h"
#include "Hardware/PllChipDb.h"

int
	main(
	int argc,
	char *argv[]
)
{
	HANDLE Driver;
	PLL_CONTEXT Context;
	int argOffset;
	BOOL hasSmbusOverride;
	USHORT smbusOverride;

	ZeroMemory(
		&Context,
		sizeof(Context)
		);

	argOffset      = 1;
	hasSmbusOverride = FALSE;
	smbusOverride  = 0;

	/* consume --smbus-base before command dispatch */
	if (argc >= 3 && !_stricmp(argv[1], "--smbus-base"))
	{
		smbusOverride    = (USHORT)ParseInt(argv[2]);
		hasSmbusOverride = TRUE;
		argOffset        = 3;
		argc            -= 2;
		argv            += 2;
	}

	if (argc < 2)
	{
		PrintUsage();
		return 1;
	}

	if (!DrvOpen(&Driver))
	{
		printf(
			"Cannot open PLLDRV (%lu)\n",
			GetLastError()
			);

		return 1;
	}

	Context.Driver = Driver;

	/* resolve all runtime paths from pllscan.cfg [Paths]; fall back to exe-relative defaults */
	{
		char exeDir[MAX_PATH];
		char cfgPath[MAX_PATH];
		char chipsetsPath[MAX_PATH];
		char boardsPath[MAX_PATH];
		char pllChipsPath[MAX_PATH];
		char boardName[128];
		BOARD_PROFILE profile;

		GetModuleFileNameA(NULL, exeDir, MAX_PATH);
		*(strrchr(exeDir, '\\') + 1) = '\0';

		lstrcpyA(cfgPath, exeDir);
		lstrcatA(cfgPath, "pllscan.cfg");

		/* [Paths] overrides; empty value → exe-relative default */
		GetPrivateProfileStringA("Paths", "ChipsetsDb", "", chipsetsPath, MAX_PATH, cfgPath);
		if (chipsetsPath[0] == '\0')
		{
			lstrcpyA(chipsetsPath, exeDir);
			lstrcatA(chipsetsPath, "Chipsets.ini");
		}

		GetPrivateProfileStringA("Paths", "BoardsDb", "", boardsPath, MAX_PATH, cfgPath);
		if (boardsPath[0] == '\0')
		{
			lstrcpyA(boardsPath, exeDir);
			lstrcatA(boardsPath, "Boards.ini");
		}

		GetPrivateProfileStringA("Paths", "PllChipsDb", "", pllChipsPath, MAX_PATH, cfgPath);
		if (pllChipsPath[0] == '\0')
		{
			lstrcpyA(pllChipsPath, exeDir);
			lstrcatA(pllChipsPath, "PllChips.ini");
		}

		ChipsetDbLoad(chipsetsPath);
		PllChipDbLoad(pllChipsPath);
		BoardDbLoad(boardsPath, cfgPath);

		GetPrivateProfileStringA(
			"Selection", "Board", "",
			boardName, sizeof(boardName),
			cfgPath);

		if (boardName[0] != '\0' && BoardDbLookup(boardName, &profile))
		{
			Context.Platform.PllType    = profile.PllType;
			Context.Platform.PllAddress = profile.PllAddress;
			lstrcpyA(Context.Platform.BoardName,       profile.Name);
			lstrcpyA(Context.Platform.NorthbridgeName, profile.Northbridge);
		}
	}

	if (!PlatformDetect(&Context))
	{
		if (!hasSmbusOverride)
		{
			printf("Platform detection failed\n");

			BoardDbFree();
			PllChipDbFree();
			ChipsetDbFree();
			DrvClose(Driver);

			return 1;
		}

		printf(
			"Platform detection failed -- using SMBus base override 0x%04X\n",
			smbusOverride
			);
	}

	if (hasSmbusOverride)
	{
		Context.Platform.SmbusBase = smbusOverride;
	}

	if (!_stricmp(argv[1], "pci"))
	{
		int r = CmdPci(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	if (!_stricmp(argv[1], "port"))
	{
		int r = CmdPort(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	if (!_stricmp(argv[1], "smbus"))
	{
		int r = CmdSmbus(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	if (!_stricmp(argv[1], "platform"))
	{
		int r = CmdPlatform(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	if (!_stricmp(argv[1], "pll"))
	{
		int r = CmdPll(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	if (!_stricmp(argv[1], "board"))
	{
		int r = CmdBoard(
			&Context,
			argc,
			argv);

		BoardDbFree();
		PllChipDbFree();
		ChipsetDbFree();
		DrvClose(Driver);

		return r;
	}

	PrintUsage();

	BoardDbFree();
	PllChipDbFree();
	ChipsetDbFree();
	DrvClose(Driver);

	return 1;
}