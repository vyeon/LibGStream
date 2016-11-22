#ifndef _INFOGRAPH_TYPE_GENERIC_PAGEDB_H_
#define _INFOGRAPH_TYPE_GENERIC_PAGEDB_H_

#include <infograph/type/generic/slotted_page.h>

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
    bool read_from_file(const char* filepath);

    uint64_t num_pages = 0;
};

template <typename PAGE_T, 
          template <typename ELEM_T,
                    typename = std::allocator<ELEM_T> >
                    class CONT_T = std::vector >
class pagedb
{
public:
    using page_t = PAGE_T;

    // constructor
    pagedb();

    inline pagedb_info& info();
protected:
    pagedb_info _info;
};


} // !namespace igraph

#endif // !_INFOGRAPH_PAGEDB_H_