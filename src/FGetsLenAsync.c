#include "async.h"
#include "asyncio_funcs.h"


AS_LVO STRPTR
FGetsLenAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf),
	AS_REG(d0, LONG numBytes),
	AS_REG(a2, LONG *len))
{
	UBYTE	*p;
	LONG	length = 0;

	p = ( UBYTE * ) buf;

	if( --numBytes <= 0 )
	{
		return( NULL );
	}

	while( TRUE )
	{
		UBYTE	*ptr;
		LONG	i, count;

		ptr = ( UBYTE * ) file->af_Offset;

		if( count = file->af_BytesLeft )
		{
			count = MIN( count, numBytes );

			for( i = 0; ( i < count ) && ( *ptr != '\n' ); ++i )
			{
				*p++ = *ptr++;
			}

			length += i;

			if( i < count )
			{
				*p++ = '\n';
				++i;
				length += 1;
			}

			file->af_BytesLeft -= i;
			file->af_Offset    += i;

			if( ( i >= numBytes ) || ( *( p - 1 ) == '\n' ) )
			{
				break;
			}

			numBytes -= i;
		}

		if( ReadAsync( file, p, 1 ) < 1 )
		{
			break;
		}

		--numBytes;
		++length;

		if( *p++ == '\n' )
		{
			break;
		}
	}

	*p = '\0';
	*len = length;

	if( p == ( UBYTE * ) buf )
	{
		return( NULL );
	}

	return( buf );
}


AS_LVO STRPTR
FGetsAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR buf),
	AS_REG(d0, LONG numBytes))
{
	LONG	len;

	return( FGetsLenAsync( file, buf, numBytes, &len ) );
}
