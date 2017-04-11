#ifndef _GSTREAM_CACHE_H_
#define _GSTREAM_CACHE_H_

#include <cstddef>

namespace gstream {

template <typename KeyTy>
class polymorphic_page_cache_interface {
public:
	using key_t = KeyTy;
	virtual bool init(const std::size_t capacity) noexcept = 0;
	virtual ~polymorphic_page_cache_interface() noexcept = default;
	virtual bool push(key_t key) noexcept = 0;
	virtual bool pop(key_t& out) noexcept = 0;
	virtual bool is_concurrent() noexcept = 0;
};
using page_cache_key = void*;

enum class cache_policy {
	FIFO,
	LIFO,
	LRU,
	MRU,
};

} // !namespace gstream

#endif // !_GSTREAM_CACHE_H_
