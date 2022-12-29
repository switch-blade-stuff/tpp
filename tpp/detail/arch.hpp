/*
 * Created by switchblade on 2022-12-22.
 */

#pragma once

#if defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_X86) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)
#define TPP_ARCH_X86
#elif defined(__ARM_ARCH_2__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM2
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM3
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM4T
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM5
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM6T2
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM6
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM7
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM7A
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM7R
#elif defined(__ARM_ARCH_7M__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM7M
#elif defined(__ARM_ARCH_7S__)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM7S
#elif defined(__aarch64__) || defined(_M_ARM64)
#define TPP_ARCH_ARM
#define TPP_ARCH_ARM64
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

#elif defined(TPP_ARCH_ARM) && (defined(__ARM_NEON__) || defined(__ARM_NEON))
#define TPP_HAS_NEON
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