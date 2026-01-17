#pragma once

#include <cstdint>

typedef std::int8_t  i8;
typedef std::int16_t i16;
typedef std::int32_t i32;
typedef std::int64_t i64;

typedef std::uint8_t   u8; // 255
typedef std::uint16_t u16; // 65,535
typedef std::uint32_t u32; // 4,294,967,295
typedef std::uint64_t u64; // 18,446,744,073,709,551,615

# define u8_max		(255)
# define u16_max	(65535)
# define u32_max	(4294967295U)
# define u64_max    (18446744073709551615)

typedef float f32;
typedef double d64;