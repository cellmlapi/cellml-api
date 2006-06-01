#include "Utilities.hxx"
#ifndef WIN32
#include <sys/time.h>
#endif

// wcsdup is non-standard, so use this instead...
wchar_t*
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
  GetSystemTimeAsFileTime(p);
  p += 2;
  *p++ = GetCurrentProcessId();
#endif  
  mersenne_init_by_array(key, p - key);
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long mersenne_genrand_int32(void)
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

int
CDA_objcmp(iface::XPCOM::IObject* o1, iface::XPCOM::IObject* o2)
  throw()
{
  char* s1, *s2;
  try
  {
    s1 = o1->objid();
  }
  catch (...)
  {
    s1 = NULL;
  }
  try
  {
    s2 = o2->objid();
  }
  catch (...)
  {
    s2 = NULL;
  }

  int cmp;
  if (s1 && s2)
  {
    cmp = strcmp(s1, s2);
  }
  // if we have a dead object, we can't compare them so easily, so we just
  // follow a basic rule. This can break ordering relationships, but once
  // they are dead, it is the best we can do.
  else if (!s1 && !s2)
    // Dead objects are automatically identical.
    cmp = 0;
  else if (!s1)
    // First is dead, second is alive, so treat as -1...
    cmp = -1;
  else
    cmp = 1;

  if (s1)
    free(s1);
  if (s2)
    free(s2);

  return cmp;
}
