#pragma once

#ifndef __CUSTOM_ENDIAN_H_
#define __CUSTOM_ENDIAN_H_

//#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
//#define IS_BIG_ENDIAN (*(WORD *)"\0\x2" == 0x200)

#ifdef _MSC_VER
#include <Windows.h>
#endif

#ifndef __BYTE_ORDER__
  #define __ORDER_LITTLE_ENDIAN__ 0x4321
  #define __ORDER_BIG_ENDIAN__ 0x1234
#if defined(ARCH_IA32) || defined(ARCH_IA64) || defined(ARCH_AMD64) || defined(ARCH_ALPHA) || defined(ARCH_ARM) || defined(ARCH_MIPS) || defined(_M_IX86) || \
		defined(_X86_) || defined(_ARM_) || defined(_AMD64_) || defined(_IA64_) \
		|| defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || defined(__LITTLE_ENDIAN__)
    #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
  #else
    #define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
	#if IS_BIG_ENDIAN || defined(__BIG_ENDIAN__)
       #define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
    #else
       #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
    #endif
  #endif
#endif

#if defined(_MSC_VER)
  #include<stdlib.h>
  #pragma intrinsic(_byteswap_ushort)
  #pragma intrinsic(_byteswap_ulong)
  #pragma intrinsic(_byteswap_uint64)
  #define _bswap16 _byteswap_ushort
  #define _bswap32 _byteswap_ulong
  #define _bswap64 _byteswap_uint64
#elif ICC_VERSION
  #define _bswap32 _bswap
  #define _bswap64 _bswap64
#elif defined(linux)
  #include <asm/byteorder.h>
  #if defined(__arch__swab16) && !defined(_bswap16)
  #define _bswap16 __arch__swab16
  #endif
  #if defined(__arch__swab32) && !defined(_bswap32)
  #define _bswap32 __arch__swab32
  #endif
  #if defined(__arch__swab64) && !defined(_bswap64)
  #define _bswap64 __arch__swab64
  #endif
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  // convert a little-endian number to/from native byte order.
  #define to_le16(x) _bswap16(x)
  #define to_le32(x) _bswap32(x)
  #define to_le64(x) _bswap64(x)
  // convert a big-endian number to/from native byte order.
  #define to_be16(x) (x)
  #define to_be32(x) (x)
  #define to_be64(x) (x)
#else // LITTLE_ENDIAN
// convert a little-endian number to/from native byte order.
  #define to_le16(x) (x)
  #define to_le32(x) (x)
  #define to_le64(x) (x)
  // convert a big-endian number to/from native byte order.
  #define to_be16(x) _bswap16(x)
  #define to_be32(x) _bswap32(x)
  #define to_be64(x) _bswap64(x)
#endif

#endif //__CUSTOM_ENDIAN_H_

