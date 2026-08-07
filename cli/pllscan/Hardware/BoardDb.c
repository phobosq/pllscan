#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BoardDb.h"
#include "PllChipDb.h"


static BOARD_PROFILE *g_Boards;
static int            g_BoardCount;


static char g_ClockLabels[16][16];
static int  g_ClockLabelCount;


static
	void
	LoadClockLabels(
	    const char *CfgPath
	    )
{
	char csv[256];
	char *ctx;
	char *tok;

	GetPrivateProfileStringA(
		"BoardDb",
		"ClockLabels",
		"HT,PCIe,CPURef,MemRef,FSB",
		csv,
		sizeof(csv),
		CfgPath);

	g_ClockLabelCount = 0;
	tok = strtok_s(csv, ",", &ctx);

	while (tok != NULL && g_ClockLabelCount < 16)
	{
		while (*tok == ' ' || *tok == '\t')
			tok++;

		strncpy(
			g_ClockLabels[g_ClockLabelCount],
			tok,
			sizeof(g_ClockLabels[0]) - 1);

		g_ClockLabels[g_ClockLabelCount][sizeof(g_ClockLabels[0]) - 1] = '\0';
		g_ClockLabelCount++;

		tok = strtok_s(NULL, ",", &ctx);
	}
}


BOOL
	BoardDbLoad(
	    const char *Path,
	    const char *CfgPath
	    )
{
	char sections[8192];
	char tmp[256];
	const char *p;
	int   count;
	int   i;
	int   j;
	BOARD_PROFILE *boards;

	LoadClockLabels(CfgPath);

	if (GetPrivateProfileSectionNamesA(
		sections,
		sizeof(sections),
		Path) == 0)
	{
		return FALSE;
	}

	count = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
		count++;

	if (count == 0)
		return FALSE;

	boards = (BOARD_PROFILE *)calloc(count, sizeof(BOARD_PROFILE));

	if (!boards)
		return FALSE;

	i = 0;
	for (p = sections; *p != '\0'; p += strlen(p) + 1)
	{
		BOARD_PROFILE *b = &boards[i];

		strncpy(b->Name, p, sizeof(b->Name) - 1);
		b->Name[sizeof(b->Name) - 1] = '\0';

		GetPrivateProfileStringA(p, "Chipset",     "", b->Chipset,     sizeof(b->Chipset),     Path);
		GetPrivateProfileStringA(p, "Northbridge", "", b->Northbridge, sizeof(b->Northbridge), Path);

		GetPrivateProfileStringA(p, "PllType",    "", tmp, sizeof(tmp), Path);
		PllChipKeyFromString(tmp, b->PllKey, sizeof(b->PllKey));

		GetPrivateProfileStringA(p, "PllAddress", "0", tmp, sizeof(tmp), Path);
		b->PllAddress = (UCHAR)strtoul(tmp, NULL, 16);

		/* sparse register defaults Reg00..RegFF */
		b->RegCount = 0;
		for (j = 0; j <= 0xFF && b->RegCount < 64; j++)
		{
			char key[8];
			sprintf(key, "Reg%02X", j);
			GetPrivateProfileStringA(p, key, "", tmp, sizeof(tmp), Path);
			if (tmp[0] == '\0')
				continue;
			b->Regs[b->RegCount].Index = (UCHAR)j;
			b->Regs[b->RegCount].Value = (UCHAR)strtoul(tmp, NULL, 16);
			b->RegCount++;
		}

		/* fixed clock output labels */
		b->ClockCount = 0;
		for (j = 0; j < g_ClockLabelCount && b->ClockCount < 16; j++)
		{
			GetPrivateProfileStringA(p, g_ClockLabels[j], "", tmp, sizeof(tmp), Path);
			if (tmp[0] == '\0')
				continue;
			strncpy(
				b->Clocks[b->ClockCount].Label,
				g_ClockLabels[j],
				sizeof(b->Clocks[0].Label) - 1);
			b->Clocks[b->ClockCount].FreqMhz = strtoul(tmp, NULL, 10);
			b->ClockCount++;
		}

		i++;
	}

	BoardDbFree();

	g_Boards     = boards;
	g_BoardCount = i;

	return TRUE;
}


void
	BoardDbFree(
	    void
	    )
{
	if (!g_Boards)
		return;

	free(g_Boards);

	g_Boards     = NULL;
	g_BoardCount = 0;
}


BOOL
	BoardDbLookup(
	    const char    *Name,
	    BOARD_PROFILE *Out
	    )
{
	int i;

	for (i = 0; i < g_BoardCount; i++)
	{
		if (_stricmp(g_Boards[i].Name, Name) == 0)
		{
			*Out = g_Boards[i];
			return TRUE;
		}
	}

	return FALSE;
}


int
	BoardDbCount(
	    void
	    )
{
	return g_BoardCount;
}


BOOL
	BoardDbGetNames(
	    const char **NamesOut,
	    int          MaxNames,
	    int         *CountOut
	    )
{
	int i;
	int n;

	if (!NamesOut || !CountOut)
		return FALSE;

	n = g_BoardCount < MaxNames ? g_BoardCount : MaxNames;

	for (i = 0; i < n; i++)
		NamesOut[i] = g_Boards[i].Name;

	*CountOut = n;

	return n > 0;
}
