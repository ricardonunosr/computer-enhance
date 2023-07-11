#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <cstdint>

typedef uint32_t b32;
typedef double f64;
typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

struct buffer {
  u8 *data;
  size_t size;
};

struct haversine_pair {
  f64 x0, y0, x1, y1;
};

#endif // COMMON_H
