#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
WriteLineAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, STRPTR line))
{
#if defined( NOEXTERNALS ) && !defined( __SAS )
	LONG	i = 0;
	STRPTR	s = line;

	while( *s )
	{
		++i, ++s;
	}

	return( WriteAsync( file, line, i ) );
#else
	return( WriteAsync( file, line, strlen( line ) ) );
#endif
}
