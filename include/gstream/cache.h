#ifndef _GSTREAM_CACHE_H_
#define _GSTREAM_CACHE_H_

#include <memory>
#include <functional>
#include <gstream/gstreamdef.h>

namespace gstream {

class page_cache_policy {
public:
	using key_type = gstream_pid_t;
	using shared_ptr = std::shared_ptr<page_cache_policy>;
	using unique_ptr = std::unique_ptr<page_cache_policy>;
	using weak_ptr = std::weak_ptr<page_cache_policy>;
	virtual ~page_cache_policy() noexcept = default;
	virtual void hit(key_type key) noexcept = 0;
	virtual void push(key_type key) noexcept = 0;
	virtual void pop(key_type* out) noexcept = 0;
};

template <typename Policy>
page_cache_policy::unique_ptr page_cache_policy_generator_template(gstream_pid_t pid_min, gstream_pid_t pid_max, std::size_t capacity) {
	return std::make_unique<Policy>(pid_min, pid_max, capacity);
}

#define GSTREAM_PAGE_CACHE_POLICY_GENERATOR(__POLICY) page_cache_policy_generator_template<__POLICY>

using page_cache_policy_generator = std::function<page_cache_policy::unique_ptr (gstream_pid_t /*pid_min*/, gstream_pid_t /*pid_max*/, std::size_t /*capacity*/)>;

} // !namespace gstream

#endif // !_GSTREAM_CACHE_H_
