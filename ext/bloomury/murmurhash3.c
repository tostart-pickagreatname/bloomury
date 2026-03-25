#include "murmurhash3.h"
#include <string.h>

/* ============================================================
   MurmurHash3 — 32 bit variant
   Austin Appleby, public domain
   ============================================================ */

static inline uint32_t rotl32(uint32_t x, int8_t r) {
  return (x << r) | (x >> (32 - r));
}

#define MURMUR_C1  0xcc9e2d51
#define MURMUR_C2  0x1b873593
#define MURMUR_MIX 0xe6546b64 // additive constant in block mixing step
#define MURMUR_F1  0x85ebca6b // finalization mix constants
#define MURMUR_F2  0xc2b2ae35

uint32_t murmur3_32(const uint8_t *data, size_t len, uint32_t seed) {
  uint32_t h = seed;
  size_t nblocks = len / 4;

  const uint32_t *blocks = (const uint32_t *)data;
  for (size_t i = 0; i < nblocks; i++) {
    uint32_t k;
    memcpy(&k, blocks + i, sizeof(k));

    k *= MURMUR_C1;
    k = rotl32(k, 15);
    k *= MURMUR_C2;

    h ^= k;
    h = rotl32(h, 13);
    h = h * 5 + MURMUR_MIX;
  }

  const uint8_t *tail = data + nblocks * 4;
  uint32_t k = 0;

  switch (len & 3) {
  case 3:
    k ^= tail[2] << 16;
  case 2:
    k ^= tail[1] << 8;
  case 1:
    k ^= tail[0];
    k *= MURMUR_C1;
    k = rotl32(k, 15);
    k *= MURMUR_C2;
    h ^= k;
  }

  h ^= (uint32_t)len;
  h ^= h >> 16;
  h *= MURMUR_F1;
  h ^= h >> 13;
  h *= MURMUR_F2;
  h ^= h >> 16;

  return h;
}
