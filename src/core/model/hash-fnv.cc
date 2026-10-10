
#include "hash-fnv.h"

#include "log.h"

#include <stdlib.h>
#include <sys/types.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("Hash-Fnv");

namespace Hash {

namespace Function {

namespace Fnv1aImplementation {

extern "C" {
// NOLINTBEGIN
// clang-format off




#if !defined(__FNV_H__)
#define __FNV_H__



#define FNV_VERSION "5.0.2"


typedef uint32_t Fnv32_t;


#define FNV0_32_INIT ((Fnv1aImplementation::Fnv32_t)0)


#define FNV1_32_INIT ((Fnv1aImplementation::Fnv32_t)0x811c9dc5)
#define FNV1_32A_INIT FNV1_32_INIT


#define HAVE_64BIT_LONG_LONG



#if defined(HAVE_64BIT_LONG_LONG)
typedef uint64_t Fnv64_t;
#else
typedef struct {
    uint32_t w32[2];
} Fnv64_t;
#endif


#if defined(HAVE_64BIT_LONG_LONG)
#define FNV0_64_INIT ((Fnv1aImplementation::Fnv64_t)0)
#else
extern const Fnv64_t fnv0_64_init;
#define FNV0_64_INIT (Fnv1aImplementation::fnv0_64_init)
#endif


#if defined(HAVE_64BIT_LONG_LONG)
#define FNV1_64_INIT ((Fnv1aImplementation::Fnv64_t)0xcbf29ce484222325ULL)
#define FNV1A_64_INIT FNV1_64_INIT
#else
extern const fnv1_64_init;
extern const Fnv64_t fnv1a_64_init;
#define FNV1_64_INIT (fnv1_64_init)
#define FNV1A_64_INIT (fnv1a_64_init)
#endif


enum fnv_type {
    FNV_NONE = 0,
    FNV0_32 = 1,
    FNV1_32 = 2,
    FNV1a_32 = 3,
    FNV0_64 = 4,
    FNV1_64 = 5,
    FNV1a_64 = 6,
};


 Fnv32_t fnv_32_buf(void *buf, size_t len, Fnv32_t hval);
 Fnv32_t fnv_32_str(char *str, Fnv32_t hval);

 Fnv32_t fnv_32a_buf(void *buf, size_t len, Fnv32_t hashval);
 Fnv32_t fnv_32a_str(char *buf, Fnv32_t hashval);

 Fnv64_t fnv_64_buf(void *buf, size_t len, Fnv64_t hval);
 Fnv64_t fnv_64_str(char *str, Fnv64_t hval);

 Fnv64_t fnv_64a_buf(void *buf, size_t len, Fnv64_t hashval);
 Fnv64_t fnv_64a_str(char *buf, Fnv64_t hashval);



#endif






#define FNV_32_PRIME ((Fnv1aImplementation::Fnv32_t)0x01000193)


Fnv32_t
fnv_32a_buf(void *buf, size_t len, Fnv32_t hval)
{
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;

    while (bp < be) {

	hval ^= (Fnv32_t)*bp++;

#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_32_PRIME;
#else
	hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
    }

    return hval;
}


Fnv32_t
fnv_32a_str(char *str, Fnv32_t hval)
{
    unsigned char *s = (unsigned char *)str;

    while (*s) {

	hval ^= (Fnv32_t)*s++;

#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_32_PRIME;
#else
	hval += (hval<<1) + (hval<<4) + (hval<<7) + (hval<<8) + (hval<<24);
#endif
    }

    return hval;
}






#if !defined(HAVE_64BIT_LONG_LONG)
const Fnv64_t fnv1a_64_init = { 0x84222325, 0xcbf29ce4 };
#endif


#if defined(HAVE_64BIT_LONG_LONG)
#define FNV_64_PRIME ((Fnv1aImplementation::Fnv64_t)0x100000001b3ULL)
#else
#define FNV_64_PRIME_LOW ((unsigned long)0x1b3)
#define FNV_64_PRIME_SHIFT (8)
#endif


Fnv64_t
fnv_64a_buf(void *buf, size_t len, Fnv64_t hval)
{
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;

#if defined(HAVE_64BIT_LONG_LONG)
    while (bp < be) {

	hval ^= (Fnv64_t)*bp++;

#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_64_PRIME;
#else
	hval += (hval << 1) + (hval << 4) + (hval << 5) +
		(hval << 7) + (hval << 8) + (hval << 40);
#endif
    }

#else

    unsigned long val[4];
    unsigned long tmp[4];

    val[0] = hval.w32[0];
    val[1] = (val[0] >> 16);
    val[0] &= 0xffff;
    val[2] = hval.w32[1];
    val[3] = (val[2] >> 16);
    val[2] &= 0xffff;

    while (bp < be) {

	val[0] ^= (unsigned long)*bp++;

	tmp[0] = val[0] * FNV_64_PRIME_LOW;
	tmp[1] = val[1] * FNV_64_PRIME_LOW;
	tmp[2] = val[2] * FNV_64_PRIME_LOW;
	tmp[3] = val[3] * FNV_64_PRIME_LOW;
	tmp[2] += val[0] << FNV_64_PRIME_SHIFT;
	tmp[3] += val[1] << FNV_64_PRIME_SHIFT;
	tmp[1] += (tmp[0] >> 16);
	val[0] = tmp[0] & 0xffff;
	tmp[2] += (tmp[1] >> 16);
	val[1] = tmp[1] & 0xffff;
	val[3] = tmp[3] + (tmp[2] >> 16);
	val[2] = tmp[2] & 0xffff;
    }

    hval.w32[1] = ((val[3]<<16) | val[2]);
    hval.w32[0] = ((val[1]<<16) | val[0]);

#endif

    return hval;
}


Fnv64_t
fnv_64a_str(char *str, Fnv64_t hval)
{
    unsigned char *s = (unsigned char *)str;

#if defined(HAVE_64BIT_LONG_LONG)

    while (*s) {

	hval ^= (Fnv64_t)*s++;

#if defined(NO_FNV_GCC_OPTIMIZATION)
	hval *= FNV_64_PRIME;
#else
	hval += (hval << 1) + (hval << 4) + (hval << 5) +
		(hval << 7) + (hval << 8) + (hval << 40);
#endif
    }

#else

    unsigned long val[4];
    unsigned long tmp[4];

    val[0] = hval.w32[0];
    val[1] = (val[0] >> 16);
    val[0] &= 0xffff;
    val[2] = hval.w32[1];
    val[3] = (val[2] >> 16);
    val[2] &= 0xffff;

    while (*s) {


	tmp[0] = val[0] * FNV_64_PRIME_LOW;
	tmp[1] = val[1] * FNV_64_PRIME_LOW;
	tmp[2] = val[2] * FNV_64_PRIME_LOW;
	tmp[3] = val[3] * FNV_64_PRIME_LOW;
	tmp[2] += val[0] << FNV_64_PRIME_SHIFT;
	tmp[3] += val[1] << FNV_64_PRIME_SHIFT;
	tmp[1] += (tmp[0] >> 16);
	val[0] = tmp[0] & 0xffff;
	tmp[2] += (tmp[1] >> 16);
	val[1] = tmp[1] & 0xffff;
	val[3] = tmp[3] + (tmp[2] >> 16);
	val[2] = tmp[2] & 0xffff;
	val[0] ^= (unsigned long)(*s++);
    }

    hval.w32[1] = ((val[3]<<16) | val[2]);
    hval.w32[0] = ((val[1]<<16) | val[0]);

#endif

    return hval;
}

// clang-format on
// NOLINTEND
}

} // namespace Fnv1aImplementation

Fnv1a::Fnv1a() { clear(); }

uint32_t Fnv1a::GetHash32(const char *buffer, const std::size_t size) {
  m_hash32 = Fnv1aImplementation::fnv_32a_buf((void *)buffer, size, m_hash32);
  return m_hash32;
}

uint64_t Fnv1a::GetHash64(const char *buffer, const std::size_t size) {
  m_hash64 = Fnv1aImplementation::fnv_64a_buf((void *)buffer, size, m_hash64);
  return m_hash64;
}

void Fnv1a::clear() {
  m_hash32 = FNV1_32A_INIT;
  m_hash64 = FNV1A_64_INIT;
}

} // namespace Function

} // namespace Hash

} // namespace ns3
