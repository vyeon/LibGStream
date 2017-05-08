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

#include <utility>
#include <functional>
#include <memory>

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64
#else
#define ENV32
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

#if defined ENV64
using target_arch_size_t = uint64_t;
#elif defined ENV32
using target_arch_size_t = uint32_t;
#endif

namespace gstream {

namespace mpl {

template<int ...>
struct sequence {
};

template<int N, int ...S>
struct sequence_generator: sequence_generator < N - 1, N - 1, S... > {
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
struct int_to_type {
    enum {
        value = I
    };
};

template <class T>
struct type_to_type {
    using real_t = T;
    template <class Arg>
    explicit type_to_type(Arg&& arg):
        value(std::forward<Arg>(arg)) {
    }
    T value;
};

template <typename T>
struct _sizeof{
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