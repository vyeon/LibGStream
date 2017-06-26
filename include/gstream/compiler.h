#ifndef _GSTREAM_COMPILER_H_
#define _GSTREAM_COMPILER_H_

#if defined(_MSC_VER)
#define GSTREAM_FRAMEWORK_TOOLCHAIN_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
#define GSTREAM_FRAMEWORK_TOOLCHAIN_GCC
#endif

#if defined(GSTREAM_FRAMEWORK_TOOLCHAIN_MSVC)
#define __builtin_expect(exp, c) (exp)
#define restrict __restrict
#elif defined(GSTREAM_FRAMEWORK_TOOLCHAIN_GCC)
#endif

namespace gstream {

} // !namespace gstream

#endif // !_GSTREAM_COMPILER_H_
