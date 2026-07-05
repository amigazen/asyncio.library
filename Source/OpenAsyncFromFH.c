#include "async.h"
#include "asyncio_funcs.h"


#ifdef ASIO_NOEXTERNALS
AS_LVO struct AsyncFile *
OpenAsyncFromFH(
	AS_REG(a0, BPTR handle),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize),
	AS_REG(a1, struct ExecBase *SysBase),
	AS_REG(a2, struct DosLibrary *DOSBase))
{
	return( AS_OpenAsyncFH( handle, mode, bufferSize, FALSE, SysBase, DOSBase ) );
}
#else
AS_LVO struct AsyncFile *
OpenAsyncFromFH(
	AS_REG(a0, BPTR handle),
	AS_REG(d0, ULONG mode),
	AS_REG(d1, LONG bufferSize))
{
	return( AS_OpenAsyncFH( handle, mode, bufferSize, FALSE ) );
}
#endif
