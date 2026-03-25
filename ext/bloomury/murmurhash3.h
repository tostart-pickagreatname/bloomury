#ifndef MURMURHASH3_H
#define MURMURHASH3_H 1

#include <stddef.h>
#include <stdint.h>

uint32_t murmur3_32(const uint8_t *data, size_t len, uint32_t seed);

#endif /* MURMURHASH3_H */
