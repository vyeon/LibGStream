#ifndef _GSTREAM_MPL_H_
#define _GSTREAM_MPL_H_

/* ---------------------------------------------------------------
**
** LibGStream - Library of GStream by InfoLab @ DGIST (https://infolab.dgist.ac.kr/)
**
** mpl.h
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

#include <cstdint>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define GSTREAM_ENV64
#else
#define GSTREAM_ENV32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64
#else
#define ENV32
#endif
#endif

#if defined GSTREAM_ENV64
using target_arch_size_t = uint64_t;
#elif defined GSTREAM_ENV32
using target_arch_size_t = uint32_t;
#endif

namespace gstream {

namespace mpl {

template<int ...>
struct sequence {
};

template<int N, int ...S>
struct sequence_generator : sequence_generator < N - 1, N - 1, S... > {
};

template<int ...S>
struct sequence_generator < 0, S... > {
    typedef sequence<S...> type;
};

template <class T>
void* pvoid_cast(T pointer) {
    auto& ptr = pointer;
    void* addr = *reinterpret_cast<void**>(&ptr);
    return addr;
}

template <int I>
struct int2type {
    enum {
        value = I
    };
};

template <bool B>
struct bool2type {
    constexpr bool value = B;
};

template <typename T>
struct _sizeof {
    enum {
        value = sizeof(T)
    };
};

template <>
struct _sizeof<void> {
    enum {
        value = 0
    };
};

} // !namespace mpl

} // !namespace gstream

#endif // !_GSTREAM_MPL_H_