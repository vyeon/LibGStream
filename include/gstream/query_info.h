#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class strategy_t {
	Performance,
	Scalability
};

namespace _bufconf {

enum class _mode {
    FixedRA_Automatic,
    FixedRA_Maunal,
    VariadicRA_Manual
};

struct buffer_config {
protected:
    _mode mode;
    union {
        std::size_t pagebuf_size;
        double pagebuf_ratio;
    };
    union {
        std::size_t wabuf_size;
        double wabuf_ratio;
    };
    union {
        std::size_t rabuf_size;
        double rabuf_ratio;
    };
};

struct bufconf_fa : protected buffer_config {
    bufconf_fa(std::size_t wabuf_size) {
        mode = _mode::FixedRA_Automatic;
        this->wabuf_size = wabuf_size;
    }
};

struct bufconf_fm : protected buffer_config {
    bufconf_fm(std::size_t pagebuf_size, std::size_t wabuf_size, std::size_t rabuf_size) {
        mode = _mode::FixedRA_Maunal;
        this->pagebuf_size = pagebuf_size;
        this->wabuf_size = wabuf_size;
        this->rabuf_size = rabuf_size;
    }
};

struct bufconf_vm : protected buffer_config {
    bufconf_vm(double pagebuf_ratio, double wabuf_ratio, double rabuf_ratio) {
        mode = _mode::VariadicRA_Manual;
        this->pagebuf_ratio = pagebuf_ratio;
        this->wabuf_ratio = wabuf_ratio;
        this->rabuf_ratio = rabuf_ratio;
    }
};

} // !namespace _bufconf

using buffer_config = _bufconf::buffer_config;
using bufconf_fa = _bufconf::bufconf_fa; // FixedRA_Automatic 
using bufconf_fm = _bufconf::bufconf_fm; // FixedRA_Maunal
using bufconf_vm = _bufconf::bufconf_vm; // VariadicRA_Manual

struct query_info {
	char const* filepath;
	std::size_t page_size;
	gstream_pid_t pid_min;
	gstream_pid_t pid_max;
	std::size_t host_pagebuf_size;
	std::size_t host_wabuf_size;
	std::size_t host_rabuf_size;
	std::size_t device_pagebuf_size;
	std::size_t device_wabuf_size;
	std::size_t device_rabuf_size;
	strategy_t	strategy;
    buffer_config bufconf;
	page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_