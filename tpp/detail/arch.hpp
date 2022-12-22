/*
 * Created by switchblade on 12/22/22.
 */

#pragma once

#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_X86) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#define TPP_ARCH_X86
#endif

#ifndef TPP_NO_SIMD

#ifdef TPP_ARCH_X86
#ifdef __SSE__
#define TPP_HAS_SSE
#if __SSE2__
#define TPP_HAS_SSE2
#endif
#endif

#ifdef __SSSE3__
#define TPP_HAS_SSSE3
#endif

#ifdef _M_IX86_FP

#if !defined(TPP_HAS_SSE) && (_M_IX86_FP >= 1 || defined(_M_AMD64) || defined(_M_X64))
#define TPP_HAS_SSE
#endif

#if !defined(TPP_HAS_SSE2) && defined(TPP_HAS_SSE) && (_M_IX86_FP >= 2 || defined(_M_AMD64) || defined(_M_X64))
#define TPP_HAS_SSE2
#endif

#endif
#endif

#endif

#ifdef _WIN32
/* Windows is always LE */
#define TPP_BYTE_ORDER_LE 0
#define TPP_BYTE_ORDER_BE 1
#define TPP_BYTE_ORDER TPP_BYTE_ORDER_LE
#else
#define TPP_BYTE_ORDER_LE __ORDER_LITTLE_ENDIAN__
#define TPP_BYTE_ORDER_BE __ORDER_BIG_ENDIAN__
#define TPP_BYTE_ORDER __BYTE_ORDER__
#endif