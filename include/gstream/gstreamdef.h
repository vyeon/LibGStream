#ifndef _GSTREAM_CONSTDEFS_H_
#define _GSTREAM_CONSTDEFS_H_

#include <cstdint>

namespace gstream {

using gstream_pid_t = uint64_t;

constexpr unsigned long long SIZE_1KiB = 1024ull;
constexpr unsigned long long SIZE_1MiB = SIZE_1KiB * 1024ull;
constexpr unsigned long long SIZE_256MiB = SIZE_1MiB * 256ull;
constexpr unsigned long long SIZE_512MiB = SIZE_1MiB * 512ull;
constexpr unsigned long long SIZE_1GiB = SIZE_1MiB * 1024ull;
constexpr unsigned long long SIZE_2GiB = SIZE_1GiB * 2ull;

} // !namespace gstream

#endif // !_GSTREAM_CONSTDEFS_H_
