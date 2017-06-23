#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class gstream_strategy {
    Performance,
    Scalability
};

namespace detail {

enum class configuration_type {
    FixedSeparatedRA,
    VariadicSeparatedRA
};

struct buffer_configuration {
    configuration_type conftype;
    std::size_t total_hostbuf_size; // All
    std::size_t raunit_size; // FixedSeparatedRA 
    union {
        std::size_t host_rabuf_size; // FixedSeparatedRA
        double host_rabuf_ratio; // VariadicSeparatedRA
    };
    union {
        std::size_t device_rabuf_size; // FixedSeparatedRA
        double device_rabuf_ratio; // VariadicSeparatedRA
    };
    std::size_t host_wabuf_size; // All
    std::size_t device_wabuf_size; // All
};

} // !namespace detail

inline detail::buffer_configuration FixedSeparatedRA(std::size_t total_hostbuf_size,
                                                     std::size_t raunit_size,
                                                     std::size_t host_rabuf_size,
                                                     std::size_t device_rabuf_size,
                                                     std::size_t host_wabuf_size,
                                                     std::size_t device_wabuf_size
) {
    detail::buffer_configuration conf;
    conf.conftype = detail::configuration_type::FixedSeparatedRA;
    conf.total_hostbuf_size = total_hostbuf_size;
    conf.raunit_size = raunit_size;
    conf.host_rabuf_size = host_rabuf_size;
    conf.device_rabuf_size = device_rabuf_size;
    conf.host_wabuf_size = host_wabuf_size;
    conf.device_wabuf_size = device_wabuf_size;
    return conf;
}

inline detail::buffer_configuration VariadicSeparatedRA(std::size_t total_hostbuf_size,
                                                        double host_rabuf_ratio,
                                                        double device_rabuf_ratio,
                                                        std::size_t host_wabuf_size,
                                                        std::size_t device_wabuf_size
) {
    detail::buffer_configuration conf;
    conf.conftype = detail::configuration_type::VariadicSeparatedRA;
    conf.total_hostbuf_size = total_hostbuf_size;
    conf.host_rabuf_ratio = host_rabuf_ratio;
    conf.device_rabuf_ratio = device_rabuf_ratio;
    conf.host_wabuf_size = host_wabuf_size;
    conf.device_wabuf_size = device_wabuf_size;
    return conf;
}

using gstream_bufconf = detail::buffer_configuration;

struct query_info {
    const char** filepathes;
    std::size_t num_disks;
    std::size_t page_size;
    gstream_pid_t pid_min;
    gstream_pid_t pid_max;
    gstream_strategy strategy;
    gstream_bufconf bufconf;
    page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_