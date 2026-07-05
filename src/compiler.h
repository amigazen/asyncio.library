/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * compiler.h - thin shim over NDK clib/compiler-specific.h
 */

#ifndef ASYNCIO_COMPILER_H
#define ASYNCIO_COMPILER_H

#include <exec/types.h>
#include <clib/compiler-specific.h>

#ifndef AS_INITTABLE_DEFINED
#define AS_INITTABLE_DEFINED 1
struct InitTable
{
	ULONG it_LibSize;
	APTR *it_FuncTable;
	APTR  it_DataTable;
	APTR  it_InitFunc;
};
#endif

#if defined(ASIO_SHARED_LIB) || defined(ASIO_REGARGS)
#define AS_REG(r, p) __REG__(r, p)
#else
#define AS_REG(r, p) p
#endif

#if defined(ASIO_SHARED_LIB)
#define AS_LVO __ASM__ __SAVE_DS__
#elif defined(ASIO_REGARGS)
#define AS_LVO __ASM__
#else
#define AS_LVO __STDARGS__
#endif

#endif /* ASYNCIO_COMPILER_H */
