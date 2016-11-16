#ifndef _INFOGRAPH_TYPE_PAGEDB_H_
#define _INFOGRAPH_TYPE_PAGEDB_H_

#include <cstdint>

namespace igraph {

class pagedb_info
{
public:
    // constructor: using a default ctor
    pagedb_info() = default;
    // constructor: read an information file when pagedb_info creation
    explicit pagedb_info(const char* filepath);
    // destructor: using a default dtor
    ~pagedb_info() = default;
    // read an information file
    bool read_from_file(const char* filepath) noexcept;

    uint64_t num_pages = 0;
};


} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_PAGEDB_H_