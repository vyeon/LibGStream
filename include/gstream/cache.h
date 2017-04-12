#ifndef _GSTREAM_CACHE_H_
#define _GSTREAM_CACHE_H_

#include <gstream/mpl.h>

namespace gstream {

template <typename KeyTy>
class polymorphic_cache_policy_interface {
public:
	using key_type = KeyTy;
	using type = polymorphic_cache_policy_interface<KeyTy>;
	using shared_ptr = std::shared_ptr<type>;
	using unique_ptr = std::unique_ptr<type>;
	using weak_ptr = std::weak_ptr<type>;
	virtual ~polymorphic_cache_policy_interface() noexcept = default;
	virtual bool hit(key_type key) noexcept = 0;
	virtual bool push(key_type key) noexcept = 0;
	virtual bool pop(key_type& out) noexcept = 0;
};

namespace page_cache {

using key_type = void*;
using cache_policy_type = polymorphic_cache_policy_interface<key_type>;

template <typename CacheReplacementPolicy>
cache_policy_type::unique_ptr page_cache_policy_allocator(std::size_t capacity, key_type hint) {
	static_assert(std::is_base_of<cache_policy_type, CacheReplacementPolicy>::value, "Invalid Page Cache Replacement Policy.");
	return mpl::make_unique<CacheReplacementPolicy>(capacity);
}

} // !namespace page_cache

} // !namespace gstream

#endif // !_GSTREAM_CACHE_H_
