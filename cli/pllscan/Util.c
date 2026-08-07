#include <stdio.h>
#include <stdlib.h>

#include "Util.h"

ULONG
	ParseInt(
	const char *Text
	)
{
	return strtoul(
		Text,
		NULL,
		0
		);
}


ULONG
	ParseHex(
	const char *Text
	)
{
	return strtoul(
		Text,
		NULL,
		16
		);
}


void
	PrintUsage(
	void
	)
{
	printf("PLLScan\n");
	printf("\n");

	printf("PCI:\n");
	printf("  pllscan pci scan\n");
	printf("  pllscan pci read <bus> <dev> <fn> <reg>\n");

	printf("\n");

	printf("PORT:\n");
	printf("  pllscan port read <port>\n");
	printf("  pllscan port write <port> <value>\n");

	printf("\n");

	printf("SMBUS:\n");
	printf("  pllscan smbus read <addr> <cmd>\n");
	printf("  pllscan smbus write <addr> <cmd> <val> --force\n");
	printf("  pllscan smbus scan\n");

	printf("\n");

	printf("PLL:\n");
	printf("  pllscan pll probe\n");
	printf("  pllscan pll dump <addr>\n");
	printf("  pllscan pll decode <addr>\n");

	printf("\n");

	printf("BOARD:\n");
	printf("  pllscan board list\n");
	printf("  pllscan board select <name>\n");
	printf("  pllscan board show\n");
	printf("  pllscan board clear\n");
}