#ifndef _UTILITIES_HXX
#define _UTILITIES_HXX

#include "cda_compiler_support.h"
#include "ThreadWrapper.hxx"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if SIZEOF_WCHAR_TP == 8
#define WCHAR_T_IS_64BIT
#define WCHAR_T_CONSTANT_WIDTH
#elif SIZEOF_WCHAR_TP == 4
#define WCHAR_T_IS_32BIT
#define WCHAR_T_CONSTANT_WIDTH
#elif SIZEOF_WCHAR_TP != 2
#error "Only UTF16 and UCS4 wide characters are supported, but your "
#error "compiler has a different length type."
#endif

#if defined(__GNUC__) && (__GNUC__ > 2)
#define CDA_LIKELY(x) (__builtin_expect(!!(x), 1))
#define CDA_UNLIKELY(x) (__builtin_expect(x, 0))
#define CDA_PURE __attribute__((pure))
#else
#define CDA_LIKELY(x) (x)
#define CDA_UNLIKELY(x) (x)
#define CDA_PURE
#endif

#include <list>
#include <exception>
#include <Ifacexpcom.hxx>
#include <wchar.h>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#else
#include <windows.h>
#undef CreateEvent
#undef GetObject
#undef GetClassName
#undef AddMonitor
#endif

#include <stdlib.h>
#include <string.h>

#include <locale.h>

template<typename T>
class XPCOMContainerReleaser
{
public:
  XPCOMContainerReleaser(T& aCont)
    : mCont(aCont), mOverride(false)
  {}

  ~XPCOMContainerReleaser()
  {
    if (mOverride)
      return;
    for (typename T::iterator i = mCont.begin();
         i != mCont.end(); i++)
      (*i)->release_ref();
  }

  void override()
  {
    mOverride = true;
  }

private:
  T& mCont;
  bool mOverride;
};

template<typename C>
class destructor_functor
{
public:
  virtual void operator()(C& aValue) = 0;
};

template<typename C>
class cxxptr_destructor
  : public destructor_functor<C*>
{
public:
  cxxptr_destructor()
  {
  }

  void
  operator()(C*& aValue)
  {
    delete aValue;
  }
};

template<typename C>
class objref_destructor
  : public destructor_functor<C*>
{
public:
  objref_destructor()
  {
  }

  void operator()(C*& aValue)
  {
    if (aValue)
      aValue->release_ref();
  }
};

template<typename C>
class container_destructor
  : public destructor_functor<C>
{
public:
  container_destructor(destructor_functor<typename C::value_type>* aSubDis)
    : mSubDis(aSubDis)
  {
  }

  ~container_destructor()
  {
    delete mSubDis;
  }

  void
  operator()(C& aValue)
  {
    for (typename C::iterator i = aValue.begin(); i != aValue.end(); i++)
      (*mSubDis)(*i);
  }
private:
  destructor_functor<typename C::value_type>* mSubDis;
};

template<typename C>
class void_destructor
  : public destructor_functor<C>
{
public:
  void_destructor() {}
  void operator()(C&) {}
};

template<typename T1, typename T2>
class pair_both_destructor
  : public destructor_functor<std::pair<T1, T2> >
{
public:
  pair_both_destructor(destructor_functor<T1>* aSubDis1, destructor_functor<T2>* aSubDis2)
    : mSubDis1(aSubDis1), mSubDis2(aSubDis2)
  {
  }

  ~pair_both_destructor()
  {
    delete mSubDis1;
    delete mSubDis2;
  }

  void
  operator()(std::pair<T1, T2>& aValue)
  {
    (*mSubDis1)(aValue.first);
    (*mSubDis2)(aValue.second);
  }

private:
  destructor_functor<T1>* mSubDis1;
  destructor_functor<T2>* mSubDis2;
};

template<typename C>
class scoped_destroy
{
public:
  scoped_destroy(C& aValue, destructor_functor<C>* aDestructor)
    : mValue(aValue), mDestructor(aDestructor), mOverride(false)
  {
  }

  ~scoped_destroy()
  {
    if (!mOverride)
      (*mDestructor)(mValue);
    delete mDestructor;
  }

  void manual_destroy()
  {
    (*mDestructor)(mValue);
  }

  void override()
  {
    mOverride = true;
  }

private:
  C& mValue;
  destructor_functor<C>* mDestructor;
  bool mOverride;
};

// A dynamic_cast which can efficiently convert to the most derived type, but
// gives undefined results if T isn't the most derived type of the input.
template<class T>
class unsafe_dynamic_cast
{
public:
  template<class I>
  unsafe_dynamic_cast(I* aInput)
  {
    if (aInput == NULL)
      mTmp = NULL;
    else
      mTmp = reinterpret_cast<T>(dynamic_cast<void*>(aInput));
  }

  operator T()
  {
    return mTmp;
  }

  T get()
  {
    return mTmp;
  }

  T operator->()
  {
    return mTmp;
  }
private:
  T mTmp;
};

// wcsdup is non-standard, so use this instead...
HEADER_INLINE wchar_t*
CDA_wcsdup(const wchar_t* str)
{
  size_t l = (wcslen(str) + 1) * sizeof(wchar_t);
  wchar_t* xstr = reinterpret_cast<wchar_t*>(malloc(l));
  memcpy(xstr, str, l);
  return xstr;
}

/* 
   The following license applies to the original Mersenne Twister code. My
   modifications are Copyright (C) 2006, and are under the same license as the
   rest of the code.

   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N];
static int mti=N+1;

/* initializes mt[N] with a seed */
static void
mersenne_init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] = 
	    (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

static void
mersenne_init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    mersenne_init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
          + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
          - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

static void
mersenne_autoseed(void)
{
  unsigned long key[50];
  unsigned long *p = key;
  memset(key, 0, sizeof(key));
#ifndef WIN32
  *p++ = getuid();
  *p++ = getpid();
  struct timeval tv;
  gettimeofday(&tv, NULL);
  *p++ = tv.tv_sec;
  *p++ = tv.tv_usec;
  gethostname((char*)p, (p - key) * 4);
  key[49] = 0;
  uint32_t l = strlen((char*)p);
  p += (l>>2) + ((l & 3) ? 1 : 0);
#else
  GetSystemTimeAsFileTime((LPFILETIME)p);
  p += 2;
  *p++ = GetCurrentProcessId();
#endif  
  mersenne_init_by_array(key, p - key);
}

/* generates a random number on [0,0xffffffff]-interval */
HEADER_INLINE unsigned long mersenne_genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)
          /* if init_genrand() has not been called, a default initial seed is
           * used. Note that this seed has changed in the Bioengineering
           * Institute version to include the time, pid, and hostname.
           */
          mersenne_autoseed();

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk+1] & LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }
  
    y = mt[mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}
#undef N
#undef M
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK

HEADER_INLINE int
CDA_objcmp(iface::XPCOM::IObject* o1, iface::XPCOM::IObject* o2)
  throw()
{
  std::string s1(o1->objid()), s2(o2->objid());
  return strcmp(s1.c_str(), s2.c_str());
}

class CDA_ID
{
public:
  CDA_ID()
  {
    uint32_t a, b, c, d;
    a = mersenne_genrand_int32();
    b = mersenne_genrand_int32();
    c = mersenne_genrand_int32();
    d = mersenne_genrand_int32();
    mIDString[0] = (char)(((a << 1) & 0xFE) | 1);
    mIDString[1] = (char)(((a >> 6) & 0xFE) | 1);
    mIDString[2] = (char)(((a >> 13) & 0xFE) | 1);
    mIDString[3] = (char)(((a >> 20) & 0xFE) | 1);
    mIDString[4] = (char)(((a >> 26) & 0x1E) | ((b << 5) & 0x70) | 1);
    mIDString[5] = (char)(((b >> 2) & 0xFE) | 1);
    mIDString[6] = (char)(((b >> 9) & 0xFE) | 1);
    mIDString[7] = (char)(((b >> 16) & 0xFE) | 1);
    mIDString[8] = (char)(((b >> 23) & 0xFE) | 1);
    mIDString[9] = (char)(((b >> 30) & 0x2) | ((c << 2) & (0xFC)) | 1);
    mIDString[10] = (char)(((c >> 5) & 0xFE) | 1);
    mIDString[11] = (char)(((c >> 12) & 0xFE) | 1);
    mIDString[12] = (char)(((c >> 19) & 0xFE) | 1);
    mIDString[13] = (char)(((c >> 26) & 0x3E) | ((d << 6) & 0xC0) | 1);
    mIDString[14] = (char)(((d >> 1) & 0xFE) | 1);
    mIDString[15] = (char)(((d >> 8) & 0xFE) | 1);
    mIDString[16] = (char)(((d >> 15) & 0xFE) | 1);
    mIDString[17] = (char)(((d >> 22) & 0xFE) | 1);
    mIDString[18] = (char)(((d >> 29) & 0x06) | 1);
    mIDString[19] = 0;

    // This form is more readable, but too expensive to generate routinely.
    // sprintf(mIDString, "%08X-%04X-%04X-%04X-%04X%08X",
    //        a, (b & 0xFFFF), (b >> 16) & 0xFFFF,
    //        c & 0xFFFF, (c >> 16) & 0xFFFF, d);
    //mIDString[36] = 0;

  }

  std::string cloneID() const
  {
    return mIDString;
  }
private:
  char mIDString[20];
};

class CDA_RefCount
{
public:
  CDA_RefCount()
    : mRefcount(1)
  {
  }

  operator uint32_t()
  {
    CDALock l(mMutex);
    return mRefcount;
  }

  CDA_RefCount& operator=(const uint32_t& aValue)
  {
    CDALock l(mMutex);
    mRefcount = aValue;
    return *this;
  }

  void operator++()
  {
    CDALock l(mMutex);
    mRefcount++;
  }

  bool operator--()
  {
    CDALock l(mMutex);
    mRefcount--;
    return (mRefcount != 0);
  }

private:
  CDAMutex mMutex;
  uint32_t mRefcount;
};

#define CDA_IMPL_ID \
  private: \
    CDA_ID _cda_id; \
  public: \
  std::string objid() \
      throw() \
    { \
      return _cda_id.cloneID(); \
    }

#define CDA_IMPL_REFCOUNT \
  private: \
    CDA_RefCount _cda_refcount; \
  public: \
    void add_ref() \
      throw() \
    { \
      ++_cda_refcount; \
    } \
    void release_ref() \
      throw() \
    { \
      if (!--_cda_refcount) \
        delete this; \
    }

#define CDA_IMPL_QI0 \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      return v; \
    }

#define CDA_IMPL_QI1(c1) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      return v; \
    }

#define CDA_IMPL_QI2(c1, c2) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      return v; \
    }

#define CDA_IMPL_QI3(c1, c2, c3) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      return v; \
    }

#define CDA_IMPL_QI4(c1, c2, c3, c4) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      else if (id == #c4) \
      { \
        add_ref(); \
        return static_cast< iface::c4* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      v.push_back(#c4); \
      return v; \
    }

#define CDA_IMPL_QI5(c1, c2, c3, c4, c5) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      else if (id == #c4) \
      { \
        add_ref(); \
        return static_cast< iface::c4* >(this); \
      } \
      else if (id == #c5) \
      { \
        add_ref(); \
        return static_cast< iface::c5* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      v.push_back(#c4); \
      v.push_back(#c5); \
      return v; \
    }

#define CDA_IMPL_QI6(c1, c2, c3, c4, c5, c6) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      else if (id == #c4) \
      { \
        add_ref(); \
        return static_cast< iface::c4* >(this); \
      } \
      else if (id == #c5) \
      { \
        add_ref(); \
        return static_cast< iface::c5* >(this); \
      } \
      else if (id == #c6) \
      { \
        add_ref(); \
        return static_cast< iface::c6* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      v.push_back(#c4); \
      v.push_back(#c5); \
      v.push_back(#c6); \
      return v; \
    }

#define CDA_IMPL_QI7(c1, c2, c3, c4, c5, c6, c7) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      else if (id == #c4) \
      { \
        add_ref(); \
        return static_cast< iface::c4* >(this); \
      } \
      else if (id == #c5) \
      { \
        add_ref(); \
        return static_cast< iface::c5* >(this); \
      } \
      else if (id == #c6) \
      { \
        add_ref(); \
        return static_cast< iface::c6* >(this); \
      } \
      else if (id == #c7) \
      { \
        add_ref(); \
        return static_cast< iface::c7* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      v.push_back(#c4); \
      v.push_back(#c5); \
      v.push_back(#c6); \
      v.push_back(#c7); \
      return v; \
    }

#define CDA_IMPL_QI8(c1, c2, c3, c4, c5, c6, c7, c8) \
    void* query_interface(const std::string& id) \
      throw(std::exception&) \
    { \
      if (id == "XPCOM::IObject") \
      { \
        add_ref(); \
        return static_cast<iface::XPCOM::IObject*>(this); \
      } \
      else if (id == #c1) \
      { \
        add_ref(); \
        return static_cast< iface::c1* >(this); \
      } \
      else if (id == #c2) \
      { \
        add_ref(); \
        return static_cast< iface::c2* >(this); \
      } \
      else if (id == #c3) \
      { \
        add_ref(); \
        return static_cast< iface::c3* >(this); \
      } \
      else if (id == #c4) \
      { \
        add_ref(); \
        return static_cast< iface::c4* >(this); \
      } \
      else if (id == #c5) \
      { \
        add_ref(); \
        return static_cast< iface::c5* >(this); \
      } \
      else if (id == #c6) \
      { \
        add_ref(); \
        return static_cast< iface::c6* >(this); \
      } \
      else if (id == #c7) \
      { \
        add_ref(); \
        return static_cast< iface::c7* >(this); \
      } \
      else if (id == #c8) \
      { \
        add_ref(); \
        return static_cast< iface::c8* >(this); \
      } \
      return NULL; \
    } \
    std::vector<std::string> supported_interfaces() throw() \
    { \
      std::vector<std::string> v; \
      v.push_back("XPCOM::IObject"); \
      v.push_back(#c1); \
      v.push_back(#c2); \
      v.push_back(#c3); \
      v.push_back(#c4); \
      v.push_back(#c5); \
      v.push_back(#c6); \
      v.push_back(#c7); \
      v.push_back(#c8); \
      return v; \
    }

template<class T>
class ObjRef
{
public:
  ObjRef()
    : mPtr(NULL)
  {
  }

  ObjRef(const ObjRef<T>& aPtr)
  {
    mPtr = aPtr.getPointer();
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  ObjRef(T* aPtr)
    : mPtr(aPtr)
  {
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  ObjRef(const already_AddRefd<T> aar)
  {
    mPtr = aar.getPointer();
  }

  ~ObjRef()
  {
    if (mPtr != NULL)
      mPtr->release_ref();
  }

  T* operator-> () const
  {
    return mPtr;
  }

  T* getPointer() const
  {
    return mPtr;
  }

  operator T* () const
  {
    return mPtr;
  }

  void operator= (T* newAssign)
  {
    if (mPtr == newAssign)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = newAssign;
    if (newAssign != NULL)
      mPtr->add_ref();
  }

  // We need these explicit forms or the default overloads the templates below.
  void operator= (const already_AddRefd<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  void operator= (const ObjRef<T>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

  template<class U>
  void operator= (const already_AddRefd<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
  }

  template<class U>
  void operator= (const ObjRef<U>& newAssign)
  {
    T* nap = newAssign.getPointer();
    if (mPtr == nap)
      return;
    if (mPtr)
      mPtr->release_ref();
    mPtr = nap;
    if (mPtr != NULL)
      mPtr->add_ref();
  }

private:
  T* mPtr;
};

template<class T, class U> bool
operator==(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() == rhs.getPointer());
}

template<class T, class U> bool
operator!=(const ObjRef<T>& lhs, const ObjRef<U>& rhs)
{
  return (lhs.getPointer() != rhs.getPointer());
}

#define RETURN_INTO_WSTRING(lhs, rhs) \
  std::wstring lhs(rhs);

#define RETURN_INTO_OBJREF(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      static_cast<type*> \
      ( \
        rhs \
      ) \
    )\
  )

#define RETURN_INTO_OBJREFD(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      dynamic_cast<type*> \
      ( \
        rhs.getPointer()				\
      ) \
    )\
  )

#define RETURN_INTO_OBJREFUD(lhs, type, rhs) \
  ObjRef<type> lhs \
  ( \
    already_AddRefd<type> \
    ( \
      unsafe_dynamic_cast<type*> \
      ( \
         rhs.getPointer()				\
      ).get() \
    )\
  )

template<class T> class WeakReference;

template<class T>
class WeakReferenceTarget
{
public:
  WeakReferenceTarget()
  {
  }

  ~WeakReferenceTarget()
  {
    typename std::list<WeakReference<T>*>::iterator i;
    for (i = mActiveReferences.begin();
         i != mActiveReferences.end();
         i++)
      (*i)->targetWasDestroyed();
  }

  void
  referenceCreated(WeakReference<T>* wr)
  {
    mActiveReferences.push_back(wr);
  }

  void
  referenceDestroyed(WeakReference<T>* wr)
  {
    mActiveReferences.remove(wr);
  }
private:
  std::list<WeakReference<T>*> mActiveReferences;
};

template<class T>
class WeakReference
{
public:
  WeakReference()
    : mReferTo(NULL)
  {
  }

  WeakReference(WeakReferenceTarget<T>* aReferTo)
    : mReferTo(aReferTo)
  {
    mReferTo->referenceCreated(this);
  }

  ~WeakReference()
  {
    if (mReferTo != NULL)
      mReferTo->referenceDestroyed(this);
  }

  void
  operator= (WeakReferenceTarget<T>* aReferTo)
  {
    if (aReferTo == mReferTo)
      return;

    if (mReferTo != NULL)
      mReferTo->referenceDestroyed(this);

    mReferTo = aReferTo;
    mReferTo->referenceCreated(this);
  }

  void
  targetWasDestroyed()
  {
    mReferTo = NULL;
  }

  T* operator() ()
  {
    return static_cast<T*>(mReferTo);
  }
private:
  WeakReferenceTarget<T>* mReferTo;
};

#define QUERY_INTERFACE(lhs, rhs, type) \
  if (rhs != NULL) \
  { \
    void* _qicast_obj = rhs->query_interface(#type); \
    if (_qicast_obj != NULL) \
    { \
      lhs = already_AddRefd<iface::type>(reinterpret_cast<iface::type*>(_qicast_obj)); \
    } \
    else \
      lhs = NULL; \
  } \
  else \
    lhs = NULL;

#define QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  QUERY_INTERFACE(lhs, rhs, type) \
  if (rhs != NULL) \
  { \
    rhs->release_ref(); \
    rhs = NULL; \
  }

#define DECLARE_QUERY_INTERFACE(lhs, rhs, type) \
  iface::type* lhs; \
  QUERY_INTERFACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_REPLACE(lhs, rhs, type) \
  iface::type* lhs; \
  QUERY_INTERFACE_REPLACE(lhs, rhs, type)

#define DECLARE_QUERY_INTERFACE_OBJREF(lhs, rhs, type) \
  ObjRef<iface::type> lhs; \
  QUERY_INTERFACE(lhs, rhs, type)

/**
 * Compares two objects, o1 and o2.
 * Returns: 0 if o1 == o2
 *         <0 if o2 < o1
 *         >0 if o2 > o1
 */
int CDA_objcmp(iface::XPCOM::IObject* o1, iface::XPCOM::IObject* o2)
  throw();

struct XPCOMComparator
{
  bool
  operator()(
             iface::XPCOM::IObject* o1,
             iface::XPCOM::IObject* o2
            ) const
  {
    /* In the strict ordering, NULL < x unless x == NULL. */
    if (o1 == NULL)
      return o2 ? true : false;
    return (CDA_objcmp(o1, o2) < 0);
  }
};

wchar_t* CDA_wcsdup(const wchar_t* str);

// XXX multithreading - I don't think there is an easy way around this however,
//     unless we are going to avoid the C library, because locale is a global
//     setting.
class CNumericLocale
{
public:
  CNumericLocale()
  {
    mOldLocale = setlocale(LC_NUMERIC, "C");
  }

  ~CNumericLocale()
  {
    if (mOldLocale != NULL)
      setlocale(LC_NUMERIC, mOldLocale);
  }

private:
  const char* mOldLocale;
};

// A non-threadsafe, compilation-unit local number for serial allocation...
static uint32_t cuNextSerial = 0;

#undef DEBUG_PTR_TAG
#ifdef DEBUG_PTR_TAG
#endif

#ifdef DEBUG_PTR_TAG
#include <map>
#include <assert.h>
extern std::map<void*,uint32_t> currentlyActivePtrTags;
#endif

// Like a pointer, but with deterministic sort...
template<class C>
class ptr_tag
{
public:
  ptr_tag()
    : mSerial(0), mPtr(NULL)
  {
  }

  template<class D>
  ptr_tag(const ptr_tag<D>& aOther)
    : mSerial(aOther.serial())
  {
    D* ptr = aOther;
    mPtr = ptr;

#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
      (*(currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr)))).second++;
#endif
  }

  template<class D>
  explicit ptr_tag(D* aOther)
  {
    D* ptr = aOther;
    setup(ptr);

#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
    {
      std::map<void*,uint32_t>::iterator i = currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr));
      if (i == currentlyActivePtrTags.end())
        currentlyActivePtrTags.insert(std::pair<void*,uint32_t>(reinterpret_cast<void*>(mPtr), 1));
      else
      {
        assert((*i).second == 0);
        (*i).second++;
      }
    }
#endif
  }

  template<class D>
  ptr_tag<C>&
  operator=(D* aPtr)
  {
#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
      (*(currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr)))).second--;
#endif

    setup(aPtr);

#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
    {
      std::map<void*,uint32_t>::iterator i = currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr));
      if (i == currentlyActivePtrTags.end())
        currentlyActivePtrTags.insert(std::pair<void*,uint32_t>(reinterpret_cast<void*>(mPtr), 1));
      else
      {
        assert((*i).second == 0);
        (*i).second++;
      }
    }
#endif

    return *this;
  }

  template<class D>
  ptr_tag<C>&
  operator=(const ptr_tag<D>& aOther)
  {
#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
      (*(currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr)))).second--;
#endif

    D* ptr = aOther;
    mPtr = ptr;
    mSerial = aOther.serial();

#ifdef DEBUG_PTR_TAG
    if (mPtr != NULL)
      (*(currentlyActivePtrTags.find(reinterpret_cast<void*>(mPtr)))).second++;
#endif

    return *this;
  }

  uint32_t serial() const
  {
    return mSerial;
  };

  void setup(C* aPtr)
  {
    mSerial = cuNextSerial++;
    mPtr = aPtr;
  }

  operator C*() const
  {
    return mPtr;
  }

  C*
  operator->() const
  {
    return mPtr;
  }

  template<class D>
  bool
  operator<(const ptr_tag<D>& aOther) const
  {
    return (mSerial < aOther.serial());
  }

private:
  uint32_t mSerial;
  C* mPtr;
};

#endif // _UTILITIES_HXX
