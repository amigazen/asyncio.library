#include "async.h"
#include "asyncio_funcs.h"


AS_LVO LONG
ReadAsync(
	AS_REG(a0, struct AsyncFile *file),
	AS_REG(a1, APTR buffer),
	AS_REG(d0, LONG numBytes))
{
#ifdef ASIO_NOEXTERNALS
	struct ExecBase	*SysBase = file->af_SysBase;
#endif
	LONG totalBytes = 0;
	LONG bytesArrived;

	while( numBytes > file->af_BytesLeft )
	{
		CopyMem( file->af_Offset, buffer, file->af_BytesLeft );

		numBytes		-= file->af_BytesLeft;
		buffer			=  ( APTR ) ( ( ULONG ) buffer + file->af_BytesLeft );
		totalBytes		+= file->af_BytesLeft;
		file->af_BytesLeft	=  0;

		bytesArrived = AS_WaitPacket( file );

		if( bytesArrived <= 0 )
		{
			if( bytesArrived == 0 )
			{
				return( totalBytes );
			}

			return( -1 );
		}

		AS_SendPacket( file, file->af_Buffers[ 1 - file->af_CurrentBuf ] );

		if( file->af_SeekOffset > bytesArrived )
		{
			file->af_SeekOffset = bytesArrived;
		}

		file->af_Offset		= file->af_Buffers[ file->af_CurrentBuf ] + file->af_SeekOffset;
		file->af_CurrentBuf	= 1 - file->af_CurrentBuf;
		file->af_BytesLeft	= bytesArrived - file->af_SeekOffset;
		file->af_SeekOffset	= 0;
	}

	CopyMem( file->af_Offset, buffer, numBytes );
	file->af_BytesLeft	-= numBytes;
	file->af_Offset		+= numBytes;

	return( totalBytes + numBytes );
}
