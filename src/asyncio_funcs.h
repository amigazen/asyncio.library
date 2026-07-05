/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * asyncio_funcs.h - LVO declarations for asyncio.library sources
 *
 * Register layout matches SDK/SFD/asyncio_lib.sfd and FuncTab[] in StartUp.c.
 */

#ifndef ASYNCIO_FUNCS_H
#define ASYNCIO_FUNCS_H

#include "compiler.h"

#ifndef LIBRARIES_ASYNCIO_H
#include <libraries/asyncio.h>
#endif

#ifdef ASIO_NOEXTERNALS
AS_LVO struct AsyncFile *OpenAsync(
	AS_REG(a0, STRPTR fileName),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize),
	AS_REG(a1, struct ExecBase *SysBase),
	AS_REG(a2, struct DosLibrary *DOSBase));
AS_LVO struct AsyncFile *OpenAsyncFromFH(
	AS_REG(a0, BPTR handle),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize),
	AS_REG(a1, struct ExecBase *SysBase),
	AS_REG(a2, struct DosLibrary *DOSBase));
#else
AS_LVO struct AsyncFile *OpenAsync(
	AS_REG(a0, STRPTR fileName),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize));
AS_LVO struct AsyncFile *OpenAsyncFromFH(
	AS_REG(a0, BPTR handle),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize));
#endif

AS_LVO LONG CloseAsync(AS_REG(a0, struct AsyncFile *file));
AS_LVO LONG SeekAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(d0, LONG position),
	AS_REG(d1, LONG mode));
AS_LVO LONG ReadAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, APTR buffer),
	AS_REG(d0, LONG bytes));
AS_LVO LONG WriteAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, APTR buffer),
	AS_REG(d0, LONG bytes));
AS_LVO LONG ReadCharAsync(AS_REG(a0, struct AsyncFile *file));
AS_LVO LONG WriteCharAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(d0, ULONG ch));
AS_LVO LONG ReadLineAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf),
	AS_REG(d0, LONG bytes));
AS_LVO LONG WriteLineAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf));
AS_LVO STRPTR FGetsAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf),
	AS_REG(d0, LONG bytes));
AS_LVO STRPTR FGetsLenAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf),
	AS_REG(d0, LONG bytes),
	AS_REG(a2, LONG *length));
AS_LVO LONG PeekAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, APTR buffer),
	AS_REG(d0, LONG bytes));

#endif /* ASYNCIO_FUNCS_H */
