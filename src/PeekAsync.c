#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
PeekAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, APTR buffer),
	AS_REG(d0, LONG numBytes))
{
#ifdef ASIO_NOEXTERNALS
	struct ExecBase	*SysBase = file->af_SysBase;
#endif

	if( !file->af_BytesLeft )
	{
		LONG	bytes;

		if( ( bytes = ReadAsync( file, &bytes, 1 ) ) <= 0 )
		{
			return( bytes );
		}

		--file->af_Offset;
		++file->af_BytesLeft;
	}

	numBytes = MIN( numBytes, file->af_BytesLeft );
	CopyMem( file->af_Offset, buffer, numBytes );
	return( numBytes );
}
