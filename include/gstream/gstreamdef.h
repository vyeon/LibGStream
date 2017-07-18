#ifndef _GSTREAM_CONSTDEFS_H_
#define _GSTREAM_CONSTDEFS_H_

#include <cstdint>

namespace gstream {

using gstream_pid = uint64_t;
using gstream_device_id = int;

constexpr unsigned long long SIZE_1KiB = 1024ull;
constexpr unsigned long long SIZE_1MiB = SIZE_1KiB * 1024ull;
constexpr unsigned long long SIZE_64MiB = SIZE_1MiB * 64ull;
constexpr unsigned long long SIZE_128MiB = SIZE_1MiB * 128ull;
constexpr unsigned long long SIZE_256MiB = SIZE_1MiB * 256ull;
constexpr unsigned long long SIZE_512MiB = SIZE_1MiB * 512ull;
constexpr unsigned long long SIZE_1GiB = SIZE_1MiB * 1024ull;
constexpr unsigned long long SIZE_2GiB = SIZE_1GiB * 2ull;

} // !namespace gstream

#endif // !_GSTREAM_CONSTDEFS_H_
