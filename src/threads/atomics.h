#pragma once
/*
 *      Copyright (C) 2015 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "../os.h"

#if defined(__WINDOWS__)
#include <intrin.h>
#endif

namespace P8PLATFORM
{
  ///////////////////////////////////////////////////////////////////////////
  // 32-bit atomic compare-and-swap
  // Returns previous value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long cas(volatile long *pAddr, long expectedVal, long swapVal)
  {
#if defined(HAS_BUILTIN_SYNC_VAL_COMPARE_AND_SWAP)
    return(__sync_val_compare_and_swap(pAddr, expectedVal, swapVal));
#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
    unsigned int prev;
    __asm__ __volatile__ (
      "  1:      lwarx   %0,0,%2  \n" /* Load the current value of *pAddr(%2) into prev (%0) and lock pAddr,  */
      "          cmpw     0,%0,%3 \n" /* Verify that the current value (%2) == old value (%3) */
      "          bne-     2f      \n" /* Bail if the two values are not equal [not as expected] */
      "          stwcx.  %4,0,%2  \n" /* Attempt to store swapVal (%4) value into *pAddr (%2) [p must still be reserved] */
      "          bne-    1b       \n" /* Loop if p was no longer reserved */
      "          isync            \n" /* Reconcile multiple processors [if present] */
      "  2:                       \n"
      : "=&r" (prev), "+m" (*pAddr)                   /* Outputs [prev, *pAddr] */
      : "r" (pAddr), "r" (expectedVal), "r" (swapVal) /* Inputs [pAddr, expectedVal, swapVal] */
      : "cc", "memory");                              /* Clobbers */
    return prev;

#elif defined(__arm__)
    register long prev;
    asm volatile (
      "dmb      ish            \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
      "1:                      \n"
      "ldrex    %0, [%1]       \n" // Load the current value of *pAddr(%1) into prev (%0) and lock pAddr,
      "cmp      %0,  %2        \n" // Verify that the current value (%0) == old value (%2)
      "bne      2f             \n" // Bail if the two values are not equal [not as expected]
      "strex    r1,  %3, [%1]  \n"
      "cmp      r1,  #0        \n"
      "bne      1b             \n"
      "dmb      ish            \n" // Memory barrier.
      "2:                      \n"
      : "=&r" (prev)
      : "r"(pAddr), "r"(expectedVal),"r"(swapVal)
      : "r1"
      );
    return prev;

#elif defined(__mips__)
  // TODO:
    unsigned int prev;
    #error atomic cas undefined for mips
    return prev;

#elif defined(__WINDOWS__)
    return _InterlockedCompareExchange(pAddr, swapVal, expectedVal);

#else // Linux / OSX86 (GCC)
    long prev;
    __asm__ __volatile__ (
      "lock/cmpxchg %1, %2"
      : "=a" (prev)
      : "r" (swapVal), "m" (*pAddr), "0" (expectedVal)
      : "memory" );
    return prev;

#endif
  }

#if !defined(__ppc__) && !defined(__powerpc__) && !defined(__arm__)
  ///////////////////////////////////////////////////////////////////////////
  // 64-bit atomic compare-and-swap
  // Returns previous value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long long cas2(volatile long long* pAddr, long long expectedVal, long long swapVal)
  {
  #if defined(__ppc__) || defined(__powerpc__) || defined(__arm__) || defined(__mips__) // PowerPC, ARM, and MIPS
    // Not available/required
    // Hack to allow compilation
      throw "cas2 is not implemented";

  #elif defined(__WINDOWS__)
    return _InterlockedCompareExchange64(pAddr, swapVal, expectedVal);

  #else // Linux / OSX86 (GCC)
    #if !defined (__x86_64)
    long long prev;
    __asm__ volatile (
      " push %%ebx        \n"  // We have to manually handle ebx, because PIC uses it and the compiler refuses to build anything that touches it
      " mov %4, %%ebx     \n"
      " lock/cmpxchg8b (%%esi) \n"
      " pop %%ebx"
      : "=A" (prev)
      : "c" ((unsigned long)(swapVal >> 32)), "0" (expectedVal), "S" (pAddr), "m" (swapVal)
      : "memory");
    return prev;
    #else
    // Hack to allow compilation on x86_64
    throw "cas2 is not implemented on x86_64!";
    #endif
  #endif
  }
#endif

  ///////////////////////////////////////////////////////////////////////////
  // 32-bit atomic increment
  // Returns new value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long atomic_inc(volatile long* pAddr)
  {
#if defined(HAS_BUILTIN_SYNC_ADD_AND_FETCH)
    return __sync_add_and_fetch(pAddr, 1);

#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
    long val;
    __asm__ __volatile__ (
      "sync             \n"
      "1: lwarx  %0, 0, %1 \n"
      "addic  %0, %0, 1 \n"
      "stwcx. %0, 0, %1 \n"
      "bne-   1b        \n"
      "isync"
      : "=&r" (val)
      : "r" (pAddr)
      : "cc", "xer", "memory");
    return val;

#elif defined(__arm__) && !defined(__ARM_ARCH_5__)
    register long val;
    asm volatile (
      "dmb      ish            \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
      "1:                     \n" 
      "ldrex   %0, [%1]       \n" // (val = *pAddr)
      "add     %0,  #1        \n" // (val += 1)
      "strex   r1,  %0, [%1]	\n"
      "cmp     r1,   #0       \n"
      "bne     1b             \n"
      "dmb     ish            \n" // Memory barrier.
      : "=&r" (val)
      : "r"(pAddr)
      : "r1"
      );
    return val;

#elif defined(__mips__)
  // TODO:
    long val;
    #error AtomicIncrement undefined for mips
    return val;

#elif defined(__WINDOWS__)
    return _InterlockedIncrement(pAddr);

#elif defined(__x86_64__)
    register long result;
    __asm__ __volatile__ (
      "lock/xaddq %q0, %1"
      : "=r" (result), "=m" (*pAddr)
      : "0" ((long) (1)), "m" (*pAddr));
    return *pAddr;

#else // Linux / OSX86 (GCC)
    register long reg __asm__ ("eax") = 1;
    __asm__ __volatile__ (
      "lock/xadd %0, %1 \n"
      "inc %%eax"
      : "+r" (reg)
      : "m" (*pAddr)
      : "memory" );
    return reg;

#endif
  }

  ///////////////////////////////////////////////////////////////////////////
  // 32-bit atomic decrement
  // Returns new value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long atomic_dec(volatile long* pAddr)
  {
#if defined(HAS_BUILTIN_SYNC_SUB_AND_FETCH)
    return __sync_sub_and_fetch(pAddr, 1);

#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
    long val;
    __asm__ __volatile__ (
      "sync                \n"
  "1: lwarx  %0, 0, %1     \n"
      "addic  %0, %0, -1   \n"
      "stwcx. %0, 0, %1    \n"
      "bne-   1b           \n"
      "isync"
      : "=&r" (val)
      : "r" (pAddr)
      : "cc", "xer", "memory");
    return val;

#elif defined(__arm__)
    register long val;
    asm volatile (
      "dmb      ish           \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
      "1:                     \n" 
      "ldrex   %0, [%1]       \n" // (val = *pAddr)
      "sub     %0,  #1        \n" // (val -= 1)
      "strex   r1,  %0, [%1]	\n"
      "cmp     r1,   #0       \n"
      "bne     1b             \n"
      "dmb     ish            \n" // Memory barrier.
      : "=&r" (val)
      : "r"(pAddr)
      : "r1"
      );
    return val;

#elif defined(__mips__)
  // TODO:
    long val;
    #error AtomicDecrement undefined for mips
    return val;

#elif defined(__WINDOWS__)
    return _InterlockedDecrement(pAddr);

#elif defined(__x86_64__)
    register long result;
    __asm__ __volatile__ (
      "lock/xaddq %q0, %1"
      : "=r" (result), "=m" (*pAddr)
      : "0" ((long) (-1)), "m" (*pAddr));
    return *pAddr;

#else // Linux / OSX86 (GCC)
    register long reg __asm__ ("eax") = -1;
    __asm__ __volatile__ (
      "lock/xadd %0, %1 \n"
      "dec %%eax"
      : "+r" (reg)
      : "m" (*pAddr)
      : "memory" );
    return reg;

#endif
  }

  ///////////////////////////////////////////////////////////////////////////
  // 32-bit atomic add
  // Returns new value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long atomic_add(volatile long* pAddr, long amount)
  {
#if defined(HAS_BUILTIN_SYNC_ADD_AND_FETCH)
    return __sync_add_and_fetch(pAddr, amount);

#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
    long val;
    __asm__ __volatile__ (
      "sync                 \n"
      "1: lwarx  %0, 0, %1  \n"
      "add  %0, %2, %0      \n"
      "stwcx. %0, 0, %1     \n"
      "bne-   1b            \n"
      "isync"
      : "=&r" (val)
      : "r" (pAddr), "r" (amount)
      : "cc", "memory");
    return val;

#elif defined(__arm__) && !defined(__ARM_ARCH_5__)
    register long val;
    asm volatile (
      "dmb      ish           \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
    "1:                       \n" 
      "ldrex   %0, [%1]       \n" // (val = *pAddr)
      "add     %0,  %2        \n" // (val += amount)
      "strex   r1,  %0, [%1]	\n"
      "cmp     r1,   #0       \n"
      "bne     1b             \n"
      "dmb     ish            \n" // Memory barrier.
      : "=&r" (val)
      : "r"(pAddr), "r"(amount)
      : "r1"
      );
    return val;

#elif defined(__mips__)
  // TODO:
    long val;
    #error AtomicAdd undefined for mips
    return val;

#elif defined(__WINDOWS__)
    _InterlockedExchangeAdd(pAddr, amount);
    return *pAddr;

#elif defined(__x86_64__)
    register long result;
    __asm__ __volatile__ (
      "lock/xaddq %q0, %1"
      : "=r" (result), "=m" (*pAddr)
      : "0" ((long) (amount)), "m" (*pAddr));
    return *pAddr;

#else // Linux / OSX86 (GCC)
    register long reg __asm__ ("eax") = amount;
    __asm__ __volatile__ (
      "lock/xadd %0, %1 \n"
      "dec %%eax"
      : "+r" (reg)
      : "m" (*pAddr)
      : "memory" );
    return reg;

#endif
  }

  ///////////////////////////////////////////////////////////////////////////
  // 32-bit atomic subtract
  // Returns new value of *pAddr
  ///////////////////////////////////////////////////////////////////////////
  static inline long atomic_sub(volatile long* pAddr, long amount)
  {
#if defined(HAS_BUILTIN_SYNC_SUB_AND_FETCH)
    return __sync_sub_and_fetch(pAddr, amount);

#elif defined(__ppc__) || defined(__powerpc__) // PowerPC
    long val;
    amount *= -1;
    __asm__ __volatile__ (
      "sync                 \n"
      "1: lwarx  %0, 0, %1  \n"
      "add  %0, %2, %0      \n"
      "stwcx. %0, 0, %1     \n"
      "bne-   1b            \n"
      "isync"
      : "=&r" (val)
      : "r" (pAddr), "r" (amount)
      : "cc", "memory");
    return val;

#elif defined(__arm__)
    register long val;
    asm volatile (
      "dmb     ish            \n" // Memory barrier. Make sure all memory accesses appearing before this complete before any that appear after
      "1:                     \n" 
      "ldrex   %0, [%1]       \n" // (val = *pAddr)
      "sub     %0,  %2        \n" // (val -= amount)
      "strex   r1,  %0, [%1]	\n"
      "cmp     r1,   #0       \n"
      "bne     1b             \n"
      "dmb     ish            \n" // Memory barrier.
      : "=&r" (val)
      : "r"(pAddr), "r"(amount)
      : "r1"
      );
    return val;

#elif defined(__mips__)
  // TODO:
    #error AtomicSubtract undefined for mips
    return val;

#elif defined(__WINDOWS__)
    amount *= -1;
    _InterlockedExchangeAdd(pAddr, amount);
    return *pAddr;

#elif defined(__x86_64__)
    register long result;
    __asm__ __volatile__ (
      "lock/xaddq %q0, %1"
      : "=r" (result), "=m" (*pAddr)
      : "0" ((long) (-1 * amount)), "m" (*pAddr));
    return *pAddr;

#else // Linux / OSX86 (GCC)
    register long reg __asm__ ("eax") = -1 * amount;
    __asm__ __volatile__ (
      "lock/xadd %0, %1 \n"
      "dec %%eax"
      : "+r" (reg)
      : "m" (*pAddr)
      : "memory" );
    return reg;

#endif

  }
}
