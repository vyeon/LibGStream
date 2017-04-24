#ifndef _GSTREAM_CACHE_H_
#define _GSTREAM_CACHE_H_

#include <gstream/mpl.h>

namespace gstream {

using page_cache_key = uint64_t;

class page_cache_policy {
public:
	using key_type = page_cache_key;
	using shared_ptr = std::shared_ptr<page_cache_policy>;
	using unique_ptr = std::unique_ptr<page_cache_policy>;
	using weak_ptr = std::weak_ptr<page_cache_policy>;
	virtual ~page_cache_policy() noexcept = default;
	virtual bool hit(key_type key) noexcept = 0;
	virtual bool push(key_type key) noexcept = 0;
	virtual bool pop(key_type& out) noexcept = 0;
};

template <typename Policy>
page_cache_policy::unique_ptr generate_page_cache_policy(page_cache_key pid_min, page_cache_key pid_max) {
	return std::make_unique<Policy>(pid_min, pid_max);
}

using page_cache_policy_generator = std::function<page_cache_policy::unique_ptr (page_cache_key /*pid_min*/, page_cache_key /*pid_max*/)>;

} // !namespace gstream

#endif // !_GSTREAM_CACHE_H_
