/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * StartUp.c - LVO trap, function table, and LibInit / LibOpen / LibClose /
 *             LibExpunge / LibReserved for asyncio.library
 *
 * FuncTab[] order MUST match SDK/SFD/asyncio_lib.sfd.  Regenerate headers
 * with tools/genheaders.sh after any SFD change.
 */

#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/resident.h>
#include <dos/dos.h>

#include <proto/exec.h>

#include "compiler.h"
#include "private/as_build.h"
#include "asyncio_funcs.h"

extern const char AS_LibName[];
extern const char AS_LibId[];

struct Library *AsyncIOBase;
struct ExecBase *SysBase;
struct Library *UtilityBase;
struct DosLibrary *DOSBase;
APTR SegList;

LONG __ASM__ LibReserved(void);
struct Library *__ASM__ __SAVE_DS__ LibInit(
	__REG__(a6, struct ExecBase *sysbase),
	__REG__(a0, APTR seglist),
	__REG__(d0, struct Library *base));
struct Library *__ASM__ __SAVE_DS__ LibOpen(
	__REG__(a6, struct Library *base));
APTR __ASM__ __SAVE_DS__ LibClose(
	__REG__(a6, struct Library *base));
APTR __ASM__ __SAVE_DS__ LibExpunge(
	__REG__(a6, struct Library *base));

static VOID FreeLib(struct Library *lib);

APTR FuncTab[];

struct InitTable InitTab = {
	(ULONG)sizeof(struct Library),
	(APTR *)FuncTab,
	(APTR)NULL,
	(APTR)LibInit
};

/*
 * Standard library vectors followed by public LVOs from asyncio_lib.sfd.
 */
APTR FuncTab[] = {
	(APTR)LibOpen,
	(APTR)LibClose,
	(APTR)LibExpunge,
	(APTR)LibReserved,
	(APTR)OpenAsync,
	(APTR)OpenAsyncFromFH,
	(APTR)CloseAsync,
	(APTR)SeekAsync,
	(APTR)ReadAsync,
	(APTR)WriteAsync,
	(APTR)ReadCharAsync,
	(APTR)WriteCharAsync,
	(APTR)ReadLineAsync,
	(APTR)WriteLineAsync,
	(APTR)FGetsAsync,
	(APTR)FGetsLenAsync,
	(APTR)PeekAsync,
	(APTR)((LONG)-1)
};

LONG
__ASM__ LibReserved(void)
{
	return 0;
}

struct Library *
__ASM__ __SAVE_DS__ LibInit(
	__REG__(a6, struct ExecBase *sysbase),
	__REG__(a0, APTR seglist),
	__REG__(d0, struct Library *lib))
{
	SysBase = sysbase;

	if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37)) &&
	    (UtilityBase = OpenLibrary("utility.library", 37)))
	{
		lib->lib_Node.ln_Type = NT_LIBRARY;
		lib->lib_Node.ln_Pri = 0;
		lib->lib_Node.ln_Name = (STRPTR)AS_LibName;
		lib->lib_Flags = LIBF_CHANGED | LIBF_SUMUSED;
		lib->lib_Version = AS_LIB_VERSION;
		lib->lib_Revision = AS_LIB_REVISION;
		lib->lib_IdString = (STRPTR)AS_LibId;
		SegList = seglist;
		AsyncIOBase = lib;
	}
	else
	{
		FreeLib(lib);
		lib = NULL;
	}

	return lib;
}

struct Library *
__ASM__ __SAVE_DS__ LibOpen(__REG__(a6, struct Library *lib))
{
	++lib->lib_OpenCnt;
	lib->lib_Flags &= ~LIBF_DELEXP;
	return lib;
}

APTR
__ASM__ __SAVE_DS__ LibClose(__REG__(a6, struct Library *lib))
{
	if (lib->lib_OpenCnt && --lib->lib_OpenCnt)
	{
		return NULL;
	}

	if (lib->lib_Flags & LIBF_DELEXP)
	{
		return LibExpunge(lib);
	}

	return NULL;
}

APTR
__ASM__ __SAVE_DS__ LibExpunge(__REG__(a6, struct Library *lib))
{
	if (lib->lib_OpenCnt)
	{
		lib->lib_Flags |= LIBF_DELEXP;
		return NULL;
	}

	Remove(&lib->lib_Node);
	FreeLib(lib);

	return SegList;
}

static VOID
FreeLib(struct Library *lib)
{
	if (DOSBase)
	{
		CloseLibrary((struct Library *)DOSBase);
		CloseLibrary(UtilityBase);
		DOSBase = NULL;
		UtilityBase = NULL;
	}

	FreeMem((UBYTE *)lib - lib->lib_NegSize, lib->lib_NegSize + lib->lib_PosSize);
	AsyncIOBase = NULL;
}
