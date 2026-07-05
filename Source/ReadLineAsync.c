#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
ReadLineAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buffer),
	AS_REG(d0, LONG bufSize))
{
	LONG	len;

	if( FGetsLenAsync( file, buffer, bufSize, &len ) )
	{
		UBYTE	*end;

		end = ( ( UBYTE * ) buffer ) + len - 1;

		if( *end != '\n' )
		{
			UBYTE	ch = 0;

			while( TRUE )
			{
				UBYTE	*ptr;
				LONG	i, count;

				ptr = ( UBYTE * ) file->af_Offset;

				if( count = file->af_BytesLeft )
				{
					for( i = 0; ( i < count ) && ( *ptr != '\n' ); ++i, ++ptr )
					{
					}

					if( i < count )
					{
						ch = '\n';
						++i;
					}

					file->af_BytesLeft -= i;
					file->af_Offset    += i;

					if( i < count )
					{
						break;
					}
				}

				if( ReadAsync( file, &ch, 1 ) < 1 )
				{
					break;
				}

				if( ch == '\n' )
				{
					break;
				}
			}

			if( ch == '\n' )
			{
				*end++ = '\n';
				*end = '\0';
			}
		}
	}

	return( len );
}
