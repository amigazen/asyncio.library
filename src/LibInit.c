/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * LibInit.c - ROMTag and version strings for asyncio.library
 *
 * Resident metadata lives here so a release bump does not force
 * recompilation of StartUp.c (CLib39x pattern).
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>

#include "compiler.h"
#include "private/as_build.h"
#include "Rev.h"

const char AS_LibName[] = "asyncio.library";
const char AS_LibId[] = "asyncio.library " VERSION " (" DATE ")\r\n";

extern struct InitTable InitTab;
extern APTR __ASM__ __SAVE_DS__ LibExpunge(__REG__(a6, struct Library *base));

struct Resident RomTag = {
	RTC_MATCHWORD,
	&RomTag,
	LibExpunge,
	RTF_AUTOINIT,
	VERNUM,
	NT_LIBRARY,
	0,
	(APTR)AS_LibName,
	(APTR)AS_LibId,
	(APTR)&InitTab
};

#ifdef __SASC
void __regargs __chkabort(void) { }
void __regargs _CXBRK(void)     { }
#endif
