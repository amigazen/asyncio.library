#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
ReadCharAsync(AS_REG(a0, struct AsyncFile *file))
{
	UBYTE	ch;

	if( file->af_BytesLeft )
	{
		ch = *file->af_Offset;
		--file->af_BytesLeft;
		++file->af_Offset;

		return( ( LONG ) ch );
	}

	if( ReadAsync( file, &ch, 1 ) > 0 )
	{
		return( ( LONG ) ch );
	}

	return( -1 );
}
