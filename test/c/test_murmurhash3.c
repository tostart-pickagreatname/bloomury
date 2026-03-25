#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "murmurhash3.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(desc, expr)                                     \
  do {                                                       \
    tests_run++;                                             \
    if (expr) {                                              \
      tests_passed++;                                        \
    } else {                                                 \
      fprintf(stderr, "FAIL: %s (line %d)\n", desc, __LINE__); \
    }                                                        \
  } while (0)

int main(void) {
  /* known output values */
  TEST("empty string seed 0 = 0x00000000",
       murmur3_32((uint8_t *)"", 0, 0) == 0x00000000);
  TEST("empty string seed 1 = 0x514e28b7",
       murmur3_32((uint8_t *)"", 0, 1) == 0x514e28b7);
  TEST("\"hello\" seed 0 = 0x248bfa47",
       murmur3_32((uint8_t *)"hello", 5, 0) == 0x248bfa47);
  TEST("\"hello\" seed 0x9747b28c = 0x5d7f56e8",
       murmur3_32((uint8_t *)"hello", 5, 0x9747b28c) == 0x5d7f56e8);
  TEST("\"hello\" seed 0x5a4afe17 = 0x2c78a9d7",
       murmur3_32((uint8_t *)"hello", 5, 0x5a4afe17) == 0x2c78a9d7);

  /* determinism */
  TEST("same input same output",
       murmur3_32((uint8_t *)"hello", 5, 0) == murmur3_32((uint8_t *)"hello", 5, 0));

  /* seed sensitivity */
  TEST("different seeds produce different hashes",
       murmur3_32((uint8_t *)"hello", 5, 1) != murmur3_32((uint8_t *)"hello", 5, 2));

  /* input sensitivity */
  TEST("\"hello\" != \"Hello\"",
       murmur3_32((uint8_t *)"hello", 5, 0) != murmur3_32((uint8_t *)"Hello", 5, 0));
  TEST("length matters: \"hello\" != \"hell\"",
       murmur3_32((uint8_t *)"hello", 5, 0) != murmur3_32((uint8_t *)"hell", 4, 0));

  printf("%d/%d tests passed\n", tests_passed, tests_run);
  return tests_passed == tests_run ? 0 : 1;
}
