#ifndef _GSTREAM_QUERY_INFO_H_
#define _GSTREAM_QUERY_INFO_H_

#include <gstream/cache.h>

namespace gstream {

enum class gstream_strategy {
    Performance,
    Scalability
};

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

struct gstream_bufconfig {
    enum class policy_t {
        FixedSeparatedRA,
        /* VariadicSeparatedRA mode is not supported by current LibGStream version */
        //VariadicSeparatedRA
    };
    policy_t policy;
    std::size_t total_hostbuf_size; // All
    std::size_t topology_page_size; // All
    std::size_t ra_page_size; // FixedSeparatedRA 
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

inline gstream_bufconfig FixedSeparatedRA(std::size_t total_hostbuf_size,
                                          std::size_t topology_page_size,
                                          std::size_t ra_page_size,
                                          std::size_t host_rabuf_size,
                                          std::size_t device_rabuf_size,
                                          std::size_t host_wabuf_size,
                                          std::size_t device_wabuf_size,
                                          std::size_t device_usrspace_size
) {
    gstream_bufconfig conf;
    conf.policy = gstream_bufconfig::policy_t::FixedSeparatedRA;
    conf.total_hostbuf_size = total_hostbuf_size;
    conf.topology_page_size = topology_page_size;
    conf.ra_page_size = ra_page_size;
    conf.host_rabuf_size = host_rabuf_size;
    conf.device_rabuf_size = device_rabuf_size;
    conf.host_wabuf_size = host_wabuf_size;
    conf.device_wabuf_size = device_wabuf_size;
    conf.usrspace_size = device_usrspace_size;
    return conf;
}

#define GSTREAM_NULL_INPUT gstream_null_input_file()

struct gstream_query {
    std::size_t disk_count;
    std::size_t tpsz; // Topology Page Size
    std::size_t rpsz; // RA Page Size 0 means variadic RA
    gstream_input_file_info pagedb;
    gstream_input_file_info radb;
    gstream_pid pid_min;
    gstream_pid pid_max;
    gstream_strategy strategy;
    gstream_bufconfig bufconf;
    gstream_cpgen cpgen;
};

} // !namespace gstream

#endif // !_GSTREAM_QUERY_INFO_H_