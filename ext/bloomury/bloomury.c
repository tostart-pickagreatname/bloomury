#include "bloomury.h"

VALUE rb_mBloomury;
VALUE rb_cBloomFilter;

/* ============================================================
   MurmurHash3 — 32 bit variant
   Austin Appleby, public domain
   ============================================================ */

static inline uint32_t rotl32(uint32_t x, int8_t r) {
  return (x << r) | (x >> (32 - r));
}

#define MURMUR_C1 0xcc9e2d51
#define MURMUR_C2 0x1b873593
#define MURMUR_MIX 0xe6546b64 // additive constant in block mixing step
#define MURMUR_F1 0x85ebca6b  // finalization mix constants
#define MURMUR_F2 0xc2b2ae35

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

/* ============================================================
   Ruby API
   ============================================================ */

static void bloom_free(void *ptr) {
  BloomFilter *f = (BloomFilter *)ptr;
  bloom_filter_free(f);
  ruby_xfree(f);
}

static size_t bloom_memsize(const void *ptr) {
  const BloomFilter *f = (const BloomFilter *)ptr;
  return sizeof(BloomFilter) + (f->bit_count + 7) / 8;
}

static const rb_data_type_t bloom_type = {"BloomFilter",
                                          {NULL, bloom_free, bloom_memsize},
                                          0,
                                          0,
                                          RUBY_TYPED_FREE_IMMEDIATELY};

static VALUE bloom_alloc(VALUE klass) {
  BloomFilter *f = ruby_xcalloc(1, sizeof(BloomFilter));
  return TypedData_Wrap_Struct(klass, &bloom_type, f);
}

static VALUE bloom_initialize(VALUE self, VALUE capacity, VALUE error_rate) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);

  double n = NUM2DBL(capacity);
  double p = NUM2DBL(error_rate);

  if (n <= 0)
    rb_raise(rb_eArgError, "capacity must be positive");
  if (p <= 0 || p >= 1)
    rb_raise(rb_eArgError, "error_rate must be between 0 and 1 (exclusive)");

  double m = ceil(-n * log(p) / (log(2) * log(2)));
  double k = round((m / n) * log(2));

  if (!isfinite(m) || m > (double)SIZE_MAX)
    rb_raise(rb_eRangeError,
             "filter parameters would require an infeasible allocation; "
             "run `rake memory_estimate[capacity,error_rate]` to check requirements first");

  bloom_filter_init(f, (uint64_t)m, (uint32_t)k);
  return self;
}

static VALUE bloom_add(VALUE self, VALUE item) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);

  StringValue(item);
  bloom_filter_add(f, (uint8_t *)RSTRING_PTR(item), RSTRING_LEN(item));
  return item;
}

static VALUE bloom_check(VALUE self, VALUE item) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);

  StringValue(item);
  int found =
      bloom_filter_check(f, (uint8_t *)RSTRING_PTR(item), RSTRING_LEN(item));
  return found ? Qtrue : Qfalse;
}

static VALUE bloom_item_count(VALUE self) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);
  return ULL2NUM(f->item_count);
}

static VALUE bloom_bit_count(VALUE self) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);
  return ULL2NUM(f->bit_count);
}

static VALUE bloom_hash_count(VALUE self) {
  BloomFilter *f;
  TypedData_Get_Struct(self, BloomFilter, &bloom_type, f);
  return UINT2NUM(f->hash_count);
}

RUBY_FUNC_EXPORTED void Init_bloomury(void) {
  rb_mBloomury = rb_define_module("Bloomury");
  rb_cBloomFilter = rb_define_class_under(rb_mBloomury, "Filter", rb_cObject);

  rb_define_alloc_func(rb_cBloomFilter, bloom_alloc);
  rb_define_method(rb_cBloomFilter, "initialize", bloom_initialize, 2);
  rb_define_method(rb_cBloomFilter, "add", bloom_add, 1);
  rb_define_method(rb_cBloomFilter, "include?", bloom_check, 1);
  rb_define_method(rb_cBloomFilter, "count", bloom_item_count, 0);
  rb_define_method(rb_cBloomFilter, "bit_count", bloom_bit_count, 0);
  rb_define_method(rb_cBloomFilter, "hash_count", bloom_hash_count, 0);
}
