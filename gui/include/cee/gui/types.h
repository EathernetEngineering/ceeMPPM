/*
 * CeeHealth
 * Copyright (C) 2025 Chloe Eather
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CEE_GUI_TYPES_H_
#define CEE_GUI_TYPES_H_

#include "cprocessordetection.h"

#ifdef __cplusplus
# include <cstddef>
# include <cstdint>
# include <type_traits>
#else
# include <assert.h>
#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef signed char cint8;
typedef unsigned char cuint8;
typedef signed short cint16;
typedef unsigned short cuint16;
typedef signed int cint32;
typedef unsigned int cuint32;
#ifdef __cplusplus
# define C_INT64_C(c) static_cast<long long>(c ## LL)
# define C_UINT64_C(c) static_cast<usigned long long>(c ## ULL)
#else
# define C_INT64_C(c) ((long long)(c ## LL))
# define C_UINT64_C(c) ((unsigned long long)(c ## ULL))
#endif
typedef long long cint64;
typedef unsigned long long cuint64;

#ifndef __cplusplus
typedef ptrdiff_t cptrdiff;
typedef ptrdiff_t csizetpye;
typedef ptrdiff_t cintptr;
typedef size_t cuintptr;

#define PRIdCPTRDIFF "td"
#define PRIiCPTRDIFF "ti"

#define PRIdCSIZETYPE "td"
#define PRIiCSIZETYPE "ti"

#define PRIdCINTPTR "td"
#define PRIiCINTPTR "ti"

#define PRIuCUINTPTR "zu"
#define PRIoCUINTPTR "zo"
#define PRIxCUINTPTR "zx"
#define PRIXCUINTPTR "zX"
#endif

#if defined(__cplusplus)
template<int> struct CIntegerForSize;
template<>    struct CIntegerForSize<1> {typedef uint8_t Unsigned; typedef int8_t Signed; };
template<>    struct CIntegerForSize<2> {typedef uint16_t Unsigned; typedef int16_t Signed; };
template<>    struct CIntegerForSize<4> {typedef uint32_t Unsigned; typedef int32_t Signed; };
template<>    struct CIntegerForSize<8> {typedef uint64_t Unsigned; typedef int64_t Signed; };
template<class T> struct CIntegerForSizeof: CIntegerForSize<sizeof(T)> {};
typedef CIntegerForSize<C_PROCESSOR_WORDSIZE>::Signed cregisterint;
typedef CIntegerForSize<C_PROCESSOR_WORDSIZE>::Unsigned cregisteruint;
typedef CIntegerForSizeof<void*>::Unsigned cuintptr;
typedef CIntegerForSizeof<void*>::Unsigned cptrdiff;
typedef cptrdiff cintptr;
using csizetype = CIntegerForSizeof<std::size_t>::Signed;

#if SIZE_MAX == 0xffffffffULL
# define PRIdCPTRDIFF "d"
# define PRIiCPTRDIFF "i"

# define PRIdCSIZETYPE "d"
# define PRIiCSIZETYPE "i"

# define PRIdCINTPTR "d"
# define PRIiCINTPTR "i"

# define PRIuCUINTPTR "u"
# define PRIoCUINTPTR "o"
# define PRIxCUINTPTR "x"
# define PRIXCUINTPTR "X"
#elif SIZE_MAX == 0xffffffffffffffffULL
# define PRIdCPTRDIFF "lld"
# define PRIiCPTRDIFF "lli"

# define PRIdCSIZETYPE "lld"
# define PRIiCSIZETYPE "lli"

# define PRIdCINTPTR "lld"
# define PRIiCINTPTR "lli"

# define PRIuCUINTPTR "llu"
# define PRIoCUINTPTR "llo"
# define PRIxCUINTPTR "llx"
# define PRIXCUINTPTR "llX"
#else
# error Unsupported platform (unknown value for SIZE_MAX)
#endif
#endif

#endif
