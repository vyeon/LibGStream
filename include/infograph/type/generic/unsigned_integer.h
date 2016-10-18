#ifndef _VEE_TYPE_GENERIC_UNSIGNED_INTEGER_
#define _VEE_TYPE_GENERIC_UNSIGNED_INTEGER_

#include <type_traits>
#include <cinttypes>

//#define VEE_ENABLE_FUNCTION_TRACE

namespace {

#ifdef VEE_ENABLE_FUNCTION_TRACE
#include <stdio.h>
#define VEE_FUNCTION_TRACE() \
printf("TRACE: ");\
puts(__FUNCSIG__)
#else
#define VEE_FUNCTION_TRACE()
#endif // !VEE_ENABLE_FUNCTION_TRACE

} // !unnamed namespace

namespace vee {

namespace detail {

namespace binary_array_comparator {

template <size_t ArraySize, typename BaseIntTy = uint64_t>
class unsigned_greater_than
{
    struct padding_required
    {
    };
    struct no_padding_required
    {
    };
    struct end_of_array
    {
    };

    template <size_t Offset, size_t RemainedBytes = Offset + 1>
    bool iteration(no_padding_required)
    {
        VEE_FUNCTION_TRACE();
        /*printf("%llu > %llu = %d\n",
        (*reinterpret_cast<BaseIntTy*>(lhs + Offset - sizeof(BaseIntTy))),
        (*reinterpret_cast<BaseIntTy*>(rhs + Offset - sizeof(BaseIntTy))),
        (*reinterpret_cast<BaseIntTy*>(lhs + Offset - sizeof(BaseIntTy))) > (*reinterpret_cast<BaseIntTy*>(rhs + Offset - sizeof(BaseIntTy)))
        );*/
        if ((*reinterpret_cast<BaseIntTy*>(lhs + Offset - sizeof(BaseIntTy))) > (*reinterpret_cast<BaseIntTy*>(rhs + Offset - sizeof(BaseIntTy))))
        {
            return true;
        }
        else
        {
            using NextIterTy =
                typename std::conditional<
                (RemainedBytes - sizeof(BaseIntTy)) == 0,
                end_of_array,
                typename std::conditional<
                ((RemainedBytes - sizeof(BaseIntTy)) >= sizeof(BaseIntTy)),
                no_padding_required,
                padding_required
                >::type
                >::type;
            using NextOffsetTy =
                typename std::conditional<
                ((RemainedBytes - sizeof(BaseIntTy)) >= sizeof(BaseIntTy)),
                std::integral_constant<size_t, Offset - sizeof(BaseIntTy)>,
                std::integral_constant<size_t, Offset>
                >::type;
            return iteration<NextOffsetTy::value>(NextIterTy{});
        }
    }

    template <size_t Offset, size_t RemainedBytes = Offset + 1>
    bool iteration(padding_required)
    {
        VEE_FUNCTION_TRACE();
        BaseIntTy temp_lhs;
        memset(&temp_lhs, 0, sizeof(BaseIntTy));
        memmove(&temp_lhs, lhs, RemainedBytes);

        BaseIntTy temp_rhs;
        memset(&temp_rhs, 0, sizeof(BaseIntTy));
        memmove(&temp_rhs, rhs, RemainedBytes);

        return temp_lhs > temp_rhs;
    }

    template <int Unused>
    static bool iteration(end_of_array)
    {
        VEE_FUNCTION_TRACE();
        // nothing to do.
        return false;
    }

public:
    unsigned_greater_than(void* lhs_, void* rhs_):
        lhs(reinterpret_cast<uint8_t*>(lhs_)),
        rhs(reinterpret_cast<uint8_t*>(rhs_))
    {
        // Nothing to do
    }

    bool operator()()
    {
        static_assert(ArraySize > 1, "Target type size must be bigger than zero");
        constexpr size_t IntialOffset = ArraySize - 1;
        using NextIterTy =
            typename std::conditional<
            (ArraySize - sizeof(BaseIntTy)) == 0,
            end_of_array,
            typename std::conditional<
            ((ArraySize - sizeof(BaseIntTy)) >= sizeof(BaseIntTy)),
            no_padding_required,
            padding_required
            >::type
            >::type;
        return iteration<IntialOffset>(NextIterTy{});
    }

    uint8_t* lhs;
    uint8_t* rhs;
};

} // !namespace binary_array_comparator

#pragma pack (push, 1)
template <size_t TypeSize>
class unsigned_integer_base
{
    using this_t = unsigned_integer_base<TypeSize>;
    using ref_t = this_t&;
    using rref_t = this_t&&;

protected:
    template <typename RefTy>
    ref_t assign(RefTy&& rhs)
    {
        using ValueTy = typename std::remove_reference<RefTy>::type;
        using DispatchTy = typename std::conditional<(TypeSize <= sizeof(ValueTy)), target_type_size_is_greater_or_equal_to, target_type_size_is_less>::type;
        static_assert(!std::is_floating_point<ValueTy>::value, "A generic integer can not assign from floating point type!");
        return static_cast<ref_t>(assign(std::forward<RefTy>(rhs), DispatchTy{}));
    }

    template <typename RefTy>
    bool equal_to(RefTy&& rhs)
    {
        using ValueTy = typename std::remove_reference<RefTy>::type;
        using DispatchType = typename std::conditional < TypeSize == sizeof(ValueTy), both_types_are_equal_to_in_size,
            typename std::conditional< (TypeSize > sizeof(ValueTy)), target_type_size_is_less, target_type_size_is_greater>::type >::type;
        static_assert(!std::is_floating_point<ValueTy>::value, "A generic integer can not perform \"equal_to\" operator from floating point type!");
        return equal_to(std::forward<RefTy>(rhs), DispatchType{});
    }

    bool equal_to(ref_t rhs)
    {
        VEE_FUNCTION_TRACE();
        return memcmp(this, &rhs, TypeSize) == 0;
    }

    bool equal_to(rref_t rhs)
    {
        VEE_FUNCTION_TRACE();
        return memcmp(this, &rhs, TypeSize) == 0;
    }

    struct both_types_are_equal_to_in_size
    {
    };
    struct target_type_size_is_greater
    {
    };
    struct target_type_size_is_less
    {
    };
    struct target_type_size_is_greater_or_equal_to
    {
    };
    struct target_type_size_is_less_or_equal_to
    {
    };
    struct target_type_size_is_greater_than_64bit
    {
    };
    struct target_type_size_is_less_or_equal_to_than_64bit
    {
    };
    uint8_t data[TypeSize]{ 0, }; // signed 는 size -1 bit를 데이터로 사용하는 버전으로 만들 수 있을 것 같다.

    template <
        typename RefTy,
        typename ValueTy = typename std::decay<RefTy>::type,
        size_t Offset = sizeof(ValueTy) - TypeSize,
        typename = typename std::enable_if< (TypeSize <= sizeof(ValueTy)) >::type
    >
        ref_t assign(RefTy&& rhs, target_type_size_is_greater_or_equal_to)
    {
        memmove(data, &rhs, TypeSize);
        return *this;
    }

    template <
        typename RefTy,
        typename ValueTy = typename std::decay<RefTy>::type,
        size_t Offset = TypeSize - sizeof(ValueTy),
        typename = typename std::enable_if< (TypeSize > sizeof(ValueTy)) >::type
    >
        ref_t assign(RefTy&& rhs, target_type_size_is_less)
    {
        memset(data, 0, TypeSize);
        memmove(data, &rhs, sizeof(ValueTy));
        return *this;
    }

    template <typename T,
        size_t Offset = sizeof(T) - TypeSize>
        T casting(target_type_size_is_greater_or_equal_to) const
    {
        VEE_FUNCTION_TRACE();
        T temp;
        memset(&temp, 0, sizeof(temp));
        memmove(&temp, data, TypeSize);
        return temp;
    }

    template<typename T,
        size_t Offset = TypeSize - sizeof(T)>
        T casting(target_type_size_is_less) const
    {
        VEE_FUNCTION_TRACE();
        T temp;
        memset(&temp, 0, sizeof(temp));
        memmove(&temp, data, sizeof(temp));
        return temp;
    }

    template <typename RefTy>
    bool equal_to(RefTy&& rhs, both_types_are_equal_to_in_size)
    {
        VEE_FUNCTION_TRACE();
        return memcmp(data, &rhs, TypeSize) == 0;
    }

    template <typename RefTy>
    bool equal_to(RefTy&& rhs, target_type_size_is_greater)
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<RefTy>::type;
        uint8_t temp[sizeof(ValueTy)] = { 0, };
        memmove(temp, data, TypeSize);
        return memcmp(temp, &rhs, sizeof(ValueTy)) == 0;
    }

    template <typename RefTy>
    bool equal_to(RefTy&& rhs, target_type_size_is_less)
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<RefTy>::type;
        uint8_t temp[TypeSize] = { 0, };
        memmove(&temp, &rhs, sizeof(ValueTy));
        return memcmp(data, temp, sizeof(TypeSize)) == 0;
    }
};

template <size_t TypeSize>
class unsigned_small_integer: public unsigned_integer_base<TypeSize>
{
    using base_t = unsigned_integer_base<TypeSize>;
    using this_t = unsigned_small_integer<TypeSize>;
    using ref_t = this_t&;
    using rref_t = this_t&&;
protected:
    unsigned_small_integer() = default;
    ~unsigned_small_integer() = default;

    bool greater_than(ref_t rhs)
    {
        VEE_FUNCTION_TRACE();
        uint64_t temp_lhs{ base_t::template casting<uint64_t>(typename base_t::target_type_size_is_less_or_equal_to) };
        uint64_t temp_rhs{ rhs };
        return temp_lhs > temp_rhs;
    }

    bool greater_than(rref_t rhs)
    {
        VEE_FUNCTION_TRACE();
        uint64_t temp_lhs{ base_t::template casting<uint64_t>(typename base_t::target_type_size_is_less_or_equal_to) };
        uint64_t temp_rhs{ rhs };
        return temp_lhs > temp_rhs;
    }

    template <typename RefTy>
    bool greater_than(RefTy&& rhs)
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<RefTy>::type;
        using DispatchType = typename std::conditional< (sizeof(ValueTy) <= sizeof(uint64_t)), typename base_t::target_type_size_is_less_or_equal_to_than_64bit, typename base_t::target_type_size_is_greater_than_64bit>::type;
        static_assert(!std::is_floating_point<ValueTy>::value, "A generic integer can not perform \"greater than\" operator from floating point type!");
        return greater_than(std::forward<RefTy>(rhs), DispatchType{});
    }

    //! RHS TYPE MUST BE CONVERTABLE TO uint64_t
    template <typename RefTy>
    bool greater_than(RefTy&& rhs, typename base_t::target_type_size_is_less_or_equal_to_than_64bit)
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<RefTy>::type;
        using DispatchTy = typename std::conditional<TypeSize <= sizeof(ValueTy), typename base_t::target_type_size_is_greater_or_equal_to, typename base_t::target_type_size_is_less>::type;
        uint64_t temp_lhs{ base_t::template casting<uint64_t>(DispatchTy{}) };
        uint64_t temp_rhs{ rhs };
        return temp_lhs > temp_rhs;
    }

    template <typename RefTy>
    bool greater_than(RefTy&& rhs, typename base_t::target_type_size_is_greater_than_64bit)
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<RefTy>::type;
        uint8_t temp_lhs[sizeof(ValueTy)] = { 0, };
        memmove(temp_lhs, base_t::data, TypeSize);
        binary_array_comparator::unsigned_greater_than<sizeof(ValueTy)> comparator{ &temp_lhs, &rhs };
        return comparator();
    }

};

template <size_t TypeSize>
class unsigned_large_integer: public unsigned_integer_base<TypeSize>
{
public:
    unsigned_large_integer() = default;
    ~unsigned_large_integer() = default;
};
#pragma pack (pop)

} // !namespace detail

  /* Generic N-byte unsigned integer class for Little endian system */
#pragma pack (push, 1)


template <size_t TypeSize,
    typename BaseTy = typename std::conditional<(TypeSize <= sizeof(uint64_t)), detail::unsigned_small_integer<TypeSize>, detail::unsigned_large_integer<TypeSize>>::type>
    class unsigned_integer: public BaseTy
{
public:
    using base_t = BaseTy;
    using this_t = unsigned_integer<TypeSize>;
    using ref_t = this_t&;
    using rref_t = this_t&&;
    static const int size = TypeSize;

    unsigned_integer() = default;

    template <typename T>
    unsigned_integer(T&& value)
    {
        VEE_FUNCTION_TRACE();
        this->operator=(std::forward<T>(value));
    }

    template <typename RefTy>
    ref_t operator=(RefTy&& rhs)
    {
        VEE_FUNCTION_TRACE();
        return static_cast<ref_t>(assign(std::forward<RefTy>(rhs)));
    }

    template <typename RefTy>
    bool operator==(RefTy&& rhs)
    {
        VEE_FUNCTION_TRACE();
        return (equal_to(std::forward<RefTy>(rhs)));
    }

    template <typename RefTy>
    bool operator>(RefTy&& rhs)
    {
        VEE_FUNCTION_TRACE();
        return (greater_than(std::forward<RefTy>(rhs)));
    }

    template <typename T>
    operator T() const
    {
        VEE_FUNCTION_TRACE();
        using ValueTy = typename std::remove_reference<T>::type;
        static_assert(!std::is_floating_point<ValueTy>::value, "A generic integer can not cast to floating point type!");
        static_assert(std::is_pod<ValueTy>::value, "A generic integer can not cast to non-POD type!");
        return base_t::template casting<T>(typename std::conditional<TypeSize <= sizeof(ValueTy), typename base_t::target_type_size_is_greater_or_equal_to, typename base_t::target_type_size_is_less>::type{});
    }

};
#pragma pack(pop)

} // !namespace vee

#endif // !_VEE_TYPE_GENERIC_UNSIGNED_INTEGER_