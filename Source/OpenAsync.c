#include "async.h"
#include "asyncio_funcs.h"


#ifdef ASIO_NOEXTERNALS
AS_LVO struct AsyncFile *
OpenAsync(
	AS_REG(a0, STRPTR fileName),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize),
	AS_REG(a1, struct ExecBase *SysBase),
	AS_REG(a2, struct DosLibrary *DOSBase))
#else
AS_LVO struct AsyncFile *
OpenAsync(
	AS_REG(a0, STRPTR fileName),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize))
#endif
{
	static const WORD PrivateOpenModes[] =
	{
		MODE_OLDFILE, MODE_NEWFILE, MODE_READWRITE
	};
	BPTR		handle;
	struct AsyncFile	*file = NULL;

	if( handle = Open( fileName, PrivateOpenModes[ mode ] ) )
	{
#ifdef ASIO_NOEXTERNALS
		file = AS_OpenAsyncFH( handle, mode, bufferSize, TRUE, SysBase, DOSBase );
#else
		file = AS_OpenAsyncFH( handle, mode, bufferSize, TRUE );
#endif

		if( !file )
		{
			Close( handle );
		}
	}

	return( file );
}
