#ifndef MURMURHASH_H
#define MURMURHASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

uint32_t Hash(const uint8_t* key, size_t len, uint32_t seed);

#endif // MURMURHASH_H
