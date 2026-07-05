/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * async.h - internal asyncio.library definitions
 */

#ifndef ASYNCIO_ASYNC_H
#define ASYNCIO_ASYNC_H

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>

#include "compiler.h"

#ifndef LIBRARIES_ASYNCIO_H
#include <libraries/asyncio.h>
#endif

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include <pragmas/exec_sysbase_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include <string.h>

#ifndef ASIO_NOEXTERNALS
extern struct DosLibrary	*DOSBase;
extern struct ExecBase		*SysBase;
#endif

#ifdef ASIO_SHARED_LIB
extern struct ExecBase		*SysBase;
extern struct Library		*UtilityBase;
extern struct DosLibrary	*DOSBase;
#endif

#define D_S(type,name) char a_##name[ sizeof( type ) + 3 ]; \
			type *name = ( type * ) ( ( LONG ) ( a_##name + 3 ) & ~3 );

#ifndef MIN
#define MIN(a,b) ( ( a ) < ( b ) ? ( a ) : ( b ) )
#endif

#ifdef ASIO_NOEXTERNALS
struct AsyncFile *
AS_OpenAsyncFH( BPTR handle, ULONG mode, LONG bufferSize, BOOL closeIt,
	struct ExecBase *SysBase, struct DosLibrary *DOSBase );
#else
struct AsyncFile *
AS_OpenAsyncFH( BPTR handle, ULONG mode, LONG bufferSize, BOOL closeIt );
#endif
VOID AS_SendPacket( struct AsyncFile *file, APTR arg2 );
LONG AS_WaitPacket( struct AsyncFile *file );
VOID AS_RequeuePacket( struct AsyncFile *file );
VOID AS_RecordSyncFailure( struct AsyncFile *file );

#endif /* ASYNCIO_ASYNC_H */
