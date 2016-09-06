#ifndef _INFOGRAPH_MPL_H_
#define _INFOGRAPH_MPL_H_

#include <utility>

namespace igraph {

namespace mpl {

template<int ...>
struct sequence
{
};

template<int N, int ...S>
struct sequence_generator: sequence_generator < N - 1, N - 1, S... >
{
};

template<int ...S>
struct sequence_generator < 0, S... >
{
    typedef sequence<S...> type;
};

template <class T>
void* pvoid_cast(T pointer)
{
    auto& ptr = pointer;
    void* addr = *static_cast<void**>(&ptr);
    return addr;
}

template <int I>
struct int_to_type
{
    enum
    {
        value = I
    };
};

template <class T>
struct type_to_type
{
    using real_type = T;
    template <class Arg>
    explicit type_to_type(Arg&& arg):
        value(std::forward<Arg>(arg))
    {
    }
    T value;
};

template <bool B>
struct binary_dispatch
{
    // static const bool value = B;
};

template< class T >
struct is_pair
{
    static const bool value = false;
};

template< class T1, class T2 >
struct is_pair< std::pair< T1, T2 > >
{
    static const bool value = true;
};

template <typename T>
struct tricky_sizeof
{
    enum
    {
        value = sizeof(T)
    };
};

template <>
struct tricky_sizeof<void>
{
    enum
    {
        value = 0
    };
};

} // !namespace mpl

} // !namespace igraph

#endif // !_INFOGRAPH_MPL_H_
