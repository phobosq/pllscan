#include <stdio.h>
#include <string.h>

#include "..\Commands.h"
#include "..\Hardware\BoardDb.h"
#include "..\Hardware\PllChipDb.h"
#include "..\Util.h"


static
	void
	GetCfgPath(
	    char *PathOut,
	    int   MaxLen
	    )
{
	char *last;

	GetModuleFileNameA(NULL, PathOut, MaxLen);
	last = strrchr(PathOut, '\\');

	if (last)
		*(last + 1) = '\0';

	lstrcatA(PathOut, "pllscan.cfg");
}


int
	CmdBoard(
	PPLL_CONTEXT Context,
	int argc,
	char *argv[]
)
{
	const char *sub;
	char cfgPath[MAX_PATH];

	if (argc < 3)
		goto usage;

	sub = argv[2];

	GetCfgPath(cfgPath, MAX_PATH);

	if (!_stricmp(sub, "list"))
	{
		const char *names[256];
		int count;
		int i;

		if (!BoardDbGetNames(names, 256, &count) || count == 0)
		{
			printf("No boards loaded (Boards.ini not found or empty)\n");
			return 0;
		}

		for (i = 0; i < count; i++)
			printf("%s\n", names[i]);

		return 0;
	}

	if (!_stricmp(sub, "select"))
	{
		if (argc < 4)
		{
			printf("Usage: pllscan board select <name>\n");
			return 1;
		}

		if (!WritePrivateProfileStringA("Selection", "Board", argv[3], cfgPath))
		{
			printf("Failed to write pllscan.cfg (%lu)\n", GetLastError());
			return 1;
		}

		printf("Selected: %s\n", argv[3]);
		return 0;
	}

	if (!_stricmp(sub, "show"))
	{
		char name[128];
		BOARD_PROFILE profile;
		int i;

		GetPrivateProfileStringA(
			"Selection", "Board", "", name, sizeof(name), cfgPath);

		if (name[0] == '\0')
		{
			printf("No board selected\n");
			return 0;
		}

		if (!BoardDbLookup(name, &profile))
		{
			printf("Board not found in Boards.ini: %s\n", name);
			return 1;
		}

		{
			PPLL_CHIP_DESC chip = PllChipByKey(profile.PllKey);

			printf("Board      : %s\n", profile.Name);
			printf("Chipset    : %s\n", profile.Chipset);
			printf("Northbridge: %s\n",
				profile.Northbridge[0] ? profile.Northbridge : "(none)");
			printf("PLL key    : %s\n", profile.PllKey[0] ? profile.PllKey : "(none)");
			printf("PLL chip   : %s\n", chip != NULL ? chip->Name : "(unknown)");
			printf("PLL address: %02Xh\n", profile.PllAddress);
		}

		if (profile.ClockCount > 0)
		{
			printf("Clocks     :\n");
			for (i = 0; i < profile.ClockCount; i++)
				printf("  %-8s  %lu MHz\n",
					profile.Clocks[i].Label,
					profile.Clocks[i].FreqMhz);
		}

		if (profile.RegCount > 0)
		{
			printf("Reg defaults: %d entries\n", profile.RegCount);
		}

		return 0;
	}

	if (!_stricmp(sub, "clear"))
	{
		WritePrivateProfileStringA("Selection", "Board", "", cfgPath);
		printf("Selection cleared\n");
		return 0;
	}

usage:
	printf("Usage:\n");
	printf("  pllscan board list\n");
	printf("  pllscan board select <name>\n");
	printf("  pllscan board show\n");
	printf("  pllscan board clear\n");
	return 1;
}
