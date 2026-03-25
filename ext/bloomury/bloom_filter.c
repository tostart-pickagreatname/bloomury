#include "bloomury.h"
#include "murmurhash3.h"

/* ============================================================
   Bitfield operations
   ============================================================ */

static inline void bit_set(uint8_t *bits, uint64_t pos) {
  bits[pos / 8] |= (1 << (pos % 8));
}

static inline int bit_get(const uint8_t *bits, uint64_t pos) {
  return (bits[pos / 8] >> (pos % 8)) & 1;
}

/* ============================================================
   BloomFilter struct operations
   ============================================================ */

void bloom_filter_init(BloomFilter *f, uint64_t bit_count,
                       uint32_t hash_count) {
  f->bit_count = bit_count;
  f->hash_count = hash_count;
  f->item_count = 0;
  f->bits = ruby_xcalloc((bit_count + 7) / 8, 1);
}

void bloom_filter_free(BloomFilter *f) {
  ruby_xfree(f->bits);
  f->bits = NULL;
}

void bloom_filter_add(BloomFilter *f, const uint8_t *data, size_t len) {
  uint32_t h1 = murmur3_32(data, len, 0x9747b28c);
  uint32_t h2 = murmur3_32(data, len, 0x5a4afe17);

  for (uint32_t i = 0; i < f->hash_count; i++) {
    uint64_t pos = ((uint64_t)h1 + i * (uint64_t)h2) % f->bit_count;
    bit_set(f->bits, pos);
  }
  f->item_count++;
}

int bloom_filter_check(BloomFilter *f, const uint8_t *data, size_t len) {
  uint32_t h1 = murmur3_32(data, len, 0x9747b28c);
  uint32_t h2 = murmur3_32(data, len, 0x5a4afe17);

  for (uint32_t i = 0; i < f->hash_count; i++) {
    uint64_t pos = ((uint64_t)h1 + i * (uint64_t)h2) % f->bit_count;
    if (!bit_get(f->bits, pos))
      return 0;
  }
  return 1;
}
