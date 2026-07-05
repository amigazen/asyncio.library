#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
WriteCharAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(d0, ULONG ch))
{
	if( file->af_BytesLeft )
	{
		*file->af_Offset = (UBYTE)ch;
		--file->af_BytesLeft;
		++file->af_Offset;

		return( 1 );
	}

	{
		TEXT	c;

		c = (TEXT)ch;

		return( WriteAsync( file, &c, 1 ) );
	}
}
