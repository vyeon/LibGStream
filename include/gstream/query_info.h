#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class strategy_t {
	Performance,
	Scalability
};

struct query_info {
	std::size_t host_pagebuf_size;
	std::size_t host_wabuf_size;
	std::size_t host_rabuf_size;
	std::size_t device_pagebuf_size;
	std::size_t device_wabuf_size;
	std::size_t device_rabuf_size;
	strategy_t	strategy;
	page_cache_policy_generator host_cpgen;
	page_cache_policy_generator device_cpgen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_