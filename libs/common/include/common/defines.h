#pragma once

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

constexpr u64 KiB(u64 kb = 1) { return kb * 1024; }
constexpr u64 MiB(u64 mb = 1) { return KiB(mb) * 1024; }
constexpr u64 GiB(u64 gb = 1) { return MiB(gb) * 1024; }
constexpr u64 TiB(u64 tb = 1) { return GiB(tb) * 1024; }
