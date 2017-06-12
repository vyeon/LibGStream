#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class gstream_strategy {
	Performance,
	Scalability
};

namespace _bufconf {

enum class mode_t {
    FixedRA_Automatic,
    FixedRA_Maunal,
    VariadicRA_Manual
};

struct device_buffer_config {
    mode_t mode;
    union {
        std::size_t pagebuf_size;   // FixedRA_Maunal
        double pagebuf_ratio;       // VariadicRA_Manual
    }; 
    union {
        std::size_t raunit_size;    // FixedRA_Automatic
        std::size_t rabuf_size;     // FixedRA_Maunal
        double rabuf_ratio;         // VariadicRA_Manual
    };
    union {
        std::size_t wabuf_size;     // All
    };
};

struct host_buffer_config: public device_buffer_config {
    std::size_t total_size;
};

struct dev_bufconf_fraa: /*private*/ device_buffer_config {
    dev_bufconf_fraa(std::size_t raunit_size, std::size_t wabuf_size) {
        mode = mode_t::FixedRA_Automatic;
        this->raunit_size = raunit_size;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_fraa: /*private*/ host_buffer_config {
    host_bufconf_fraa(std::size_t total_size, std::size_t raunit_size, std::size_t wabuf_size) {
        mode = mode_t::FixedRA_Automatic;
        this->raunit_size = raunit_size;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    } 
};

struct dev_bufconf_fram: private device_buffer_config {
    dev_bufconf_fram(std::size_t pagebuf_size, std::size_t rabuf_size, std::size_t wabuf_size) {
        mode = mode_t::FixedRA_Maunal;
        this->pagebuf_size = pagebuf_size;
        this->rabuf_size = rabuf_size;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_fram: private host_buffer_config {
    host_bufconf_fram(std::size_t total_size, std::size_t pagebuf_size, std::size_t rabuf_size, std::size_t wabuf_size) {
        mode = mode_t::FixedRA_Maunal;
        this->pagebuf_size = pagebuf_size;
        this->rabuf_size = rabuf_size;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    }
};

struct dev_bufconf_vram : private device_buffer_config {
    dev_bufconf_vram(double pagebuf_ratio, double rabuf_ratio, std::size_t wabuf_size) {
        mode = mode_t::VariadicRA_Manual;
        this->pagebuf_ratio = pagebuf_ratio;
        this->rabuf_ratio = rabuf_ratio;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_vram: private host_buffer_config {
    host_bufconf_vram(std::size_t total_size, double pagebuf_ratio, double rabuf_ratio, std::size_t wabuf_size) {
        mode = mode_t::VariadicRA_Manual;
        this->pagebuf_ratio = pagebuf_ratio;
        this->rabuf_ratio = rabuf_ratio;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    }
};

} // !namespace _bufconf

using dev_bufconf = _bufconf::device_buffer_config;
using dev_bufconf_fraa = _bufconf::dev_bufconf_fraa; // FixedRA_Automatic 
using dev_bufconf_fram = _bufconf::dev_bufconf_fram; // FixedRA_Maunal
using dev_bufconf_vram = _bufconf::dev_bufconf_vram; // VariadicRA_Manual

using host_bufconf = _bufconf::host_buffer_config;
using host_bufconf_fraa = _bufconf::host_bufconf_fraa; // FixedRA_Automatic 
using host_bufconf_fram = _bufconf::host_bufconf_fram; // FixedRA_Maunal
using host_bufconf_vram = _bufconf::host_bufconf_vram; // VariadicRA_Manual

struct query_info {
	char const* filepath;
	std::size_t page_size;
	gstream_pid_t pid_min;
	gstream_pid_t pid_max;
    gstream_strategy strategy;
    host_bufconf host_bufconf;
    dev_bufconf  device_bufconf;
	page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_