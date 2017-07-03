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
    /* VariadicSeparatedRA mode is not supported by current LibGStream version */
    //VariadicSeparatedRA
};

struct buffer_configuration {
    configuration_type conftype;
    std::size_t total_hostbuf_size; // All
    std::size_t page_size; // All
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
    std::size_t usrspace_size; // All
};

} // !namespace detail

inline detail::buffer_configuration FixedSeparatedRA(std::size_t total_hostbuf_size,
                                                     std::size_t page_size,
                                                     std::size_t raunit_size,
                                                     std::size_t host_rabuf_size,
                                                     std::size_t device_rabuf_size,
                                                     std::size_t host_wabuf_size,
                                                     std::size_t device_wabuf_size,
                                                     std::size_t device_usrspace_size
) {
    detail::buffer_configuration conf;
    conf.conftype = detail::configuration_type::FixedSeparatedRA;
    conf.total_hostbuf_size = total_hostbuf_size;
    conf.page_size = page_size;
    conf.raunit_size = raunit_size;
    conf.host_rabuf_size = host_rabuf_size;
    conf.device_rabuf_size = device_rabuf_size;
    conf.host_wabuf_size = host_wabuf_size;
    conf.device_wabuf_size = device_wabuf_size;
    conf.usrspace_size = device_usrspace_size;
    return conf;
}

/* VariadicSeparatedRA mode is not supported by current LibGStream version */
//inline detail::buffer_configuration VariadicSeparatedRA() {
//}

struct gstream_file_info {
    std::wstring path;
    gstream_device_id disk_id;
    gstream_file_info(const wchar_t* filepath, gstream_device_id disk_id):
        path(filepath),
        disk_id(disk_id) {
        // Nothing to do.
    }
};

inline gstream_file_info generate_file_info(const wchar_t* filepath, gstream_device_id disk_id) {
    return gstream_file_info(filepath, disk_id);
}

inline gstream_file_info gstream_null_file() {
    return gstream_file_info(nullptr, -1);
}

struct gstream_input_file_info {
    gstream_device_id  num_partitions;
    gstream_file_info* files;
};

inline gstream_input_file_info gstream_null_input_file() {
    gstream_input_file_info info;
    info.num_partitions = 0;
    info.files = nullptr;
    return info;
}

#define GSTREAM_NULL_INPUT gstream_null_input_file()

struct gstream_query_info {
    std::size_t num_disks;
    gstream_input_file_info pagedb;
    gstream_input_file_info radb;
    gstream_pid pid_min;
    gstream_pid pid_max;
    gstream_strategy strategy;
    detail::buffer_configuration bufconf;
    page_cache_policy_generator policy_gen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_