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
    FixedIntegratedRA,
    FixedSeparatedRA,
    VariadicSeparatedRA
};

struct device_buffer_config {
    mode_t mode;
    union {
        std::size_t raunit_size;    // FixedIntegratedRA & FixedSeparatedRA 
        double pagebuf_ratio;       // VariadicSeparatedRA
    }; 
    union {
        std::size_t rabuf_size;     // FixedSeparatedRA
        double rabuf_ratio;         // VariadicSeparatedRA
    };
    union {
        std::size_t wabuf_size;     // All
    };
};

struct host_buffer_config: public device_buffer_config {
    std::size_t total_size;
};

struct dev_bufconf_fira: private device_buffer_config {
    dev_bufconf_fira(std::size_t raunit_size, std::size_t wabuf_size) {
        mode = mode_t::FixedIntegratedRA;
        this->raunit_size = raunit_size;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_fira: private host_buffer_config {
    host_bufconf_fira(std::size_t total_size, std::size_t raunit_size, std::size_t wabuf_size) {
        mode = mode_t::FixedIntegratedRA;
        this->raunit_size = raunit_size;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    } 
};

struct dev_bufconf_fsra: private device_buffer_config {
    dev_bufconf_fsra(std::size_t raunit_size, std::size_t rabuf_size, std::size_t wabuf_size) {
        mode = mode_t::FixedSeparatedRA;
        this->raunit_size = raunit_size;
        this->rabuf_size = rabuf_size;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_fsra: private host_buffer_config {
    host_bufconf_fsra(std::size_t total_size, std::size_t raunit_size, std::size_t rabuf_size, std::size_t wabuf_size) {
        mode = mode_t::FixedSeparatedRA;
        this->raunit_size = raunit_size;
        this->rabuf_size = rabuf_size;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    }
};

struct dev_bufconf_vsra : private device_buffer_config {
    dev_bufconf_vsra(double pagebuf_ratio, double rabuf_ratio, std::size_t wabuf_size) {
        mode = mode_t::VariadicSeparatedRA;
        this->pagebuf_ratio = pagebuf_ratio;
        this->rabuf_ratio = rabuf_ratio;
        this->wabuf_size = wabuf_size;
    }
};

struct host_bufconf_vsra: private host_buffer_config {
    host_bufconf_vsra(std::size_t total_size, double pagebuf_ratio, double rabuf_ratio, std::size_t wabuf_size) {
        mode = mode_t::VariadicSeparatedRA;
        this->pagebuf_ratio = pagebuf_ratio;
        this->rabuf_ratio = rabuf_ratio;
        this->wabuf_size = wabuf_size;
        this->total_size = total_size;
    }
};

} // !namespace _bufconf

using dev_bufconf_type = _bufconf::device_buffer_config;
using dev_bufconf_fira = _bufconf::dev_bufconf_fira; 
using dev_bufconf_fsra = _bufconf::dev_bufconf_fsra;
using dev_bufconf_vsra = _bufconf::dev_bufconf_vsra;

using host_bufconf_type = _bufconf::host_buffer_config;
using host_bufconf_fira = _bufconf::host_bufconf_fira;
using host_bufconf_fsra = _bufconf::host_bufconf_fsra;
using host_bufconf_vsra = _bufconf::host_bufconf_vsra;

struct query_info {
	char const* filepath;
	std::size_t page_size;
	gstream_pid_t pid_min;
	gstream_pid_t pid_max;
    gstream_strategy strategy;
    host_bufconf_type* host_bufconf;
    dev_bufconf_type*  device_bufconf;
	page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_