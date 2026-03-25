#include "bloomury.h"
#include <limits.h>
#include <math.h>

VALUE rb_mBloomury;
VALUE rb_cBloomFilter;

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

  if (k < 1)
    rb_raise(rb_eArgError, "computed hash count is zero; error_rate is too loose");

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

static VALUE bloom_add_count(VALUE self) {
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
  rb_define_method(rb_cBloomFilter, "add_count", bloom_add_count, 0);
  rb_define_method(rb_cBloomFilter, "bit_count", bloom_bit_count, 0);
  rb_define_method(rb_cBloomFilter, "hash_count", bloom_hash_count, 0);
}
