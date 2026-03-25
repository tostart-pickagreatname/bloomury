#ifndef BLOOMURY_H
#define BLOOMURY_H 1

#include "ruby.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint8_t  *bits;
  uint64_t  bit_count;
  uint32_t  hash_count;
  uint64_t  item_count;
  uint32_t  seed1;
  uint32_t  seed2;
} BloomFilter;

void bloom_filter_init(BloomFilter *f, uint64_t bit_count, uint32_t hash_count,
                       uint32_t seed1, uint32_t seed2);
void bloom_filter_free(BloomFilter *f);
void bloom_filter_add(BloomFilter *f, const uint8_t *data, size_t len);
int bloom_filter_check(BloomFilter *f, const uint8_t *data, size_t len);

#endif /* BLOOMURY_H */
