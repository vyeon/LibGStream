#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class gstream_strategy {
	Performance,
	Scalability
};

namespace _buffer_configuration {

enum class _configuration_type {
    FixedIntegratedRA,
    FixedSeparatedRA,
    VariadicSeparatedRA
};

struct device_buffer_configuration {
    _configuration_type conftype;
    union {
        std::size_t rabuf_size; // FixedSeparatedRA
        double rabuf_ratio;     // VariadicSeparatedRA
    };
    std::size_t raunit_size;    // FixedIntegratedRA & FixedSeparatedRA 
    std::size_t wabuf_size;     // All
};

struct host_buffer_configuration: public device_buffer_configuration {
    std::size_t total_size;
};

} // !namespace _bufconf

namespace device_buffer_configuration {

inline _buffer_configuration::device_buffer_configuration FixedIntegratedRA(std::size_t raunit_size, std::size_t wabuf_size) {
    _buffer_configuration::device_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::FixedIntegratedRA;
    conf.raunit_size = raunit_size;
    conf.wabuf_size = wabuf_size;
    return conf;
}

inline _buffer_configuration::device_buffer_configuration FixedSeparatedRA(std::size_t rabuf_size, std::size_t raunit_size, std::size_t wabuf_size) {
    _buffer_configuration::device_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::FixedSeparatedRA;
    conf.rabuf_size = rabuf_size;
    conf.raunit_size = raunit_size;
    conf.wabuf_size = wabuf_size;
    return conf;
}

inline _buffer_configuration::device_buffer_configuration VariadicSeparatedRA(double rabuf_ratio, std::size_t wabuf_size) {
    _buffer_configuration::device_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::VariadicSeparatedRA;
    conf.rabuf_ratio = rabuf_ratio;
    conf.wabuf_size = wabuf_size;
    return conf;
}

} // !namespace device_buffer_configuration

namespace host_buffer_configuration {

inline _buffer_configuration::host_buffer_configuration FixedIntegratedRA(std::size_t total_size, std::size_t raunit_size, std::size_t wabuf_size) {
    _buffer_configuration::host_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::FixedIntegratedRA;
    conf.raunit_size = raunit_size;
    conf.wabuf_size = wabuf_size;
    conf.total_size = total_size;
    return conf;
}

inline _buffer_configuration::host_buffer_configuration FixedSeparatedRA(std::size_t total_size, std::size_t rabuf_size, std::size_t raunit_size, std::size_t wabuf_size) {
    _buffer_configuration::host_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::FixedSeparatedRA;
    conf.rabuf_size = rabuf_size;
    conf.raunit_size = raunit_size;
    conf.wabuf_size = wabuf_size;
    conf.total_size = total_size;
    return conf;
}

inline _buffer_configuration::host_buffer_configuration VariadicSeparatedRA(std::size_t total_size, double rabuf_ratio, std::size_t wabuf_size) {
    _buffer_configuration::host_buffer_configuration conf;
    conf.conftype = _buffer_configuration::_configuration_type::VariadicSeparatedRA;
    conf.rabuf_ratio = rabuf_ratio;
    conf.wabuf_size = wabuf_size;
    conf.total_size = total_size;
    return conf;
}

} // !namespace host_buffer_configuration

using device_bufconf_type = _buffer_configuration::device_buffer_configuration;
using host_bufconf_type = _buffer_configuration::host_buffer_configuration;

struct query_info {
	char const* filepath;
	std::size_t page_size;
	gstream_pid_t pid_min;
	gstream_pid_t pid_max;
    gstream_strategy strategy;
    host_bufconf_type host_bufconf;
    device_bufconf_type device_bufconf;
	page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_