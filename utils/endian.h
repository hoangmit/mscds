#pragma once

#ifndef __CUSTOM_ENDIAN_H_
#define __CUSTOM_ENDIAN_H_

/**  \file

Macros and functions to swap byte orders

*/

//#define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
//#define IS_BIG_ENDIAN (*(WORD *)"\0\x2" == 0x200)

#ifndef __BYTE_ORDER__
  #define __ORDER_LITTLE_ENDIAN__ 0x1234
  #define __ORDER_BIG_ENDIAN__ 0x4321
  #if defined(ARCH_IA32) || defined(ARCH_IA64) || defined(ARCH_AMD64) || defined(ARCH_ALPHA) || defined(ARCH_ARM) || defined(ARCH_MIPS) || \
	      defined(_M_IX86) || defined(_X86_) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64) || defined(_M_ARM) || defined(_ARM_) || defined(_IA64_)  \
		  || defined(__i386__) || defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__amd64__) || defined(__LITTLE_ENDIAN__)
    #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
    #ifndef __LITTLE_ENDIAN__
      #define __LITTLE_ENDIAN__
    #endif
  #else
    #define IS_BIG_ENDIAN (!*(unsigned char *)&(uint16_t){1})
	#if IS_BIG_ENDIAN || defined(__BIG_ENDIAN__)
       #define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
       #ifndef __BIG_ENDIAN__
         #define __BIG_ENDIAN__
       #endif
    #else
       #define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
       #ifndef __LITTLE_ENDIAN__
         #define __LITTLE_ENDIAN__
       #endif
    #endif
  #endif
#elif (!defined(__LITTLE_ENDIAN__)) && (__BYTE_ORDER__== __ORDER_LITTLE_ENDIAN__)
  #define __LITTLE_ENDIAN__
#elif (!defined(__LITTLE_ENDIAN__)) && (__BYTE_ORDER__== __ORDER_BIG_ENDIAN__)
  #define __BIG_ENDIAN__
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

#if defined(__BIG_ENDIAN__)
  // convert a little-endian number to/from native byte order.
  #define to_le16(x) _bswap16(x)
  #define to_le32(x) _bswap32(x)
  #define to_le64(x) _bswap64(x)
  
  #define read_le16(x) _bswap16(x)
  #define read_le32(x) _bswap32(x)
  #define read_le64(x) _bswap64(x)
// convert a big-endian number to/from native byte order.
  #define to_be16(x) (x)
  #define to_be32(x) (x)
  #define to_be64(x) (x)

  #define read_be16(x) (x)
  #define read_be32(x) (x)
  #define read_be64(x) (x)
#else // LITTLE_ENDIAN
  // convert a little-endian number to/from native byte order.
  #define to_le16(x) (x)
  #define to_le32(x) (x)
  #define to_le64(x) (x)

  #define read_le16(x) (x)
  #define read_le32(x) (x)
  #define read_le64(x) (x)
// convert a big-endian number to/from native byte order.
  #define to_be16(x) _bswap16(x)
  #define to_be32(x) _bswap32(x)
  #define to_be64(x) _bswap64(x)

  #define read_be16(x) _bswap16(x)
  #define read_be32(x) _bswap32(x)
  #define read_be64(x) _bswap64(x)
#endif

enum Endianness {
	little_endian,
	big_endian,
	network_endian = big_endian,
#if defined(__LITTLE_ENDIAN__)
	host_endian = little_endian
#elif defined(__BIG_ENDIAN__)
	host_endian = big_endian
#else
#error "unable to determine system endianness"
#endif
};

#endif //__CUSTOM_ENDIAN_H_

