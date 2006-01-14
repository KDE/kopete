/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products 
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _common_h_
#define _common_h_

#if defined(_MSC_VER)
// warning C4355: 'this' : used in base member initializer list
#pragma warning(disable:4355)
#endif

#if defined(ENABLE_DEBUG_MALLOC) && !defined(ENABLE_DEBUG)
#define ENABLE_DEBUG 1
#endif

//////////////////////////////////////////////////////////////////////
// General Utilities
//////////////////////////////////////////////////////////////////////

#ifndef UNUSED
#define UNUSED(x) Unused(static_cast<const void *>(&x))
#define UNUSED2(x,y) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y))
#define UNUSED3(x,y,z) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z))
#define UNUSED4(x,y,z,a) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z)); Unused(static_cast<const void *>(&a))
#define UNUSED5(x,y,z,a,b) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z)); Unused(static_cast<const void *>(&a)); Unused(static_cast<const void *>(&b))
inline void Unused(const void *) { }
#endif // UNUSED

#define ARRAY_SIZE(x) (static_cast<int>((sizeof(x)/sizeof(x[0]))))

/////////////////////////////////////////////////////////////////////////////
// std:min/std:max on msvc
/////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER) && _MSC_VER <= 1200 // 1200 == VC++ 6.0

#undef min
#undef max

namespace std {


  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @return   The lesser of the parameters.
   *
   *  This is the simple classic generic implementation.  It will work on
   *  temporary expressions, since they are only evaluated once, unlike a
   *  preprocessor macro.
  */
  template<typename _Tp>
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b)
    {
      //return __b < __a ? __b : __a;
      if (__b < __a) return __b; return __a;
    }

  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @return   The greater of the parameters.
   *
   *  This is the simple classic generic implementation.  It will work on
   *  temporary expressions, since they are only evaluated once, unlike a
   *  preprocessor macro.
  */
  template<typename _Tp>
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b)
    {
      //return  __a < __b ? __b : __a;
      if (__a < __b) return __b; return __a;
    }

  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @param  comp  A @link s20_3_3_comparisons comparison functor@endlink.
   *  @return   The lesser of the parameters.
   *
   *  This will work on temporary expressions, since they are only evaluated
   *  once, unlike a preprocessor macro.
  */
  template<typename _Tp, typename _Compare>
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      //return __comp(__b, __a) ? __b : __a;
      if (__comp(__b, __a)) return __b; return __a;
    }

  /**
   *  @brief This does what you think it does.
   *  @param  a  A thing of arbitrary type.
   *  @param  b  Another thing of arbitrary type.
   *  @param  comp  A @link s20_3_3_comparisons comparison functor@endlink.
   *  @return   The greater of the parameters.
   *
   *  This will work on temporary expressions, since they are only evaluated
   *  once, unlike a preprocessor macro.
  */
  template<typename _Tp, typename _Compare>
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {
      //return __comp(__a, __b) ? __b : __a;
      if (__comp(__a, __b)) return __b; return __a;
    }

}

#endif // _MSC_VER <= 1200


/////////////////////////////////////////////////////////////////////////////
// Assertions
/////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_DEBUG

namespace buzz {

// Break causes the debugger to stop executing, or the program to abort
void Break();

// LogAssert writes information about an assertion to the log
void LogAssert(const char * function, const char * file, int line, const char * expression);

inline void Assert(bool result, const char * function, const char * file, int line, const char * expression) {
  if (!result) {
    LogAssert(function, file, line, expression);
    Break();
  }
}

}; // namespace buzz

#if defined(_MSC_VER) && _MSC_VER < 1300
#define __FUNCTION__ ""
#endif

#define ASSERT(x) buzz::Assert((x),__FUNCTION__,__FILE__,__LINE__,#x)
#define VERIFY(x) buzz::Assert((x),__FUNCTION__,__FILE__,__LINE__,#x)

#else // !ENABLE_DEBUG

#define ASSERT(x) (void)0
#define VERIFY(x) (void)(x)

#endif // !ENABLE_DEBUG

#define COMPILE_TIME_ASSERT(expr)       char CTA_UNIQUE_NAME[expr]
#define CTA_UNIQUE_NAME                 MAKE_NAME(__LINE__)
#define CTA_MAKE_NAME(line)             MAKE_NAME2(line)
#define CTA_MAKE_NAME2(line)            constraint_ ## line

//////////////////////////////////////////////////////////////////////
// Memory leak tracking
//////////////////////////////////////////////////////////////////////

#include <sys/types.h>

#ifdef ENABLE_DEBUG_MALLOC

namespace buzz {

void * DebugAllocate(size_t size, const char * fname = 0, int line = 0);
void DebugDeallocate(void * ptr, const char * fname = 0, int line = 0);
bool LeakCheck();
bool LeakCheckU();
void LeakMarkBaseline();
void LeakClearBaseline();

}; // namespace buzz

inline void * operator new(size_t size, const char * fname, int line) { return buzz::DebugAllocate(size, fname, line); }
inline void operator delete(void * ptr, const char * fname, int line) { buzz::DebugDeallocate(ptr, fname, line); }

#if !(defined(TRACK_ARRAY_ALLOC_PROBLEM) && \
      defined(_MSC_VER) && _MSC_VER <= 1200) // 1200 == VC++ 6.0

inline void * operator new[](size_t size, const char * fname, int line) { return buzz::DebugAllocate(size, fname, line); }
inline void operator delete[](void * ptr, const char * fname, int line) { buzz::DebugDeallocate(ptr, fname, line); }

#endif // TRACK_ARRAY_ALLOC_PROBLEM


// If you put "#define new TRACK_NEW" in your .cc file after all includes, it should track the calling function name

#define TRACK_NEW new(__FILE__,__LINE__)
#define TRACK_DEL delete(__FILE__,__LINE__)

#else // !ENABLE_DEBUG_MALLOC

#define TRACK_NEW new
#define TRACK_DEL delete

#endif // !ENABLE_DEBUG_MALLOC

//////////////////////////////////////////////////////////////////////

#endif // _common_h_
