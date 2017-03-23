#ifndef _GSTREAM_DATATYPE_PAGEDB_H_
#define _GSTREAM_DATATYPE_PAGEDB_H_

/* ---------------------------------------------------------------
**
** LibGStream - Library of GStream by InfoLab @ DGIST (https://infolab.dgist.ac.kr/)
**
** pagedb.h
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

#include <gstream/datatype/slotted_page.h>
#include <vector>
#include <fstream>
#include <iterator>

namespace gstream {

#pragma pack(push, 1)
template <typename __vertex_id_t, typename __payload_t = uint64_t>
struct rid_tuple_template
{
    using payload_t = __payload_t;
    using vertex_id_t = __vertex_id_t;
    vertex_id_t start_vid;
    payload_t   payload;
};
#pragma pack(pop)

template <typename PAGE_T,
    template <typename ELEM_T,
    typename = std::allocator<ELEM_T> >
    class CONT_T = std::vector >
CONT_T<PAGE_T> read_pages(const char* filepath, const size_t bundle_of_pages = 64)
{
    using page_t = PAGE_T;
    using cont_t = CONT_T<PAGE_T>;

    // Open a file stream
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };

    // TODO: metadata implementation

    
    cont_t pages; // container for pages which will be returned.

    // Read pages
    {
        const size_t chunk_size = sizeof(page_t) * bundle_of_pages;
        std::vector<page_t> buffer;
        buffer.resize(bundle_of_pages);
        uint64_t counter = 0;
        while (true)
        {
            ++counter;
            ifs.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
            size_t extracted = ifs.gcount() / sizeof(page_t);
            if (extracted == bundle_of_pages)
            {
                std::copy(buffer.begin(), buffer.end(), std::back_inserter(pages));
            }
            else
            {
                std::copy(buffer.begin(), buffer.begin() + extracted, std::back_inserter(pages));
                break;
            }
        }
#ifndef NDEBUG
        printf("Finished read %llu pages from file %s. bundle size = %llu, total iteration: %llu\n",
               pages.size(),
               filepath,
               bundle_of_pages,
               counter);
#endif // !NDEBUG
    }

    return pages;
}

template <typename RID_TUPLE_T,
    template <typename ELEM_T,
    typename = std::allocator<ELEM_T> >
    class CONT_T = std::vector >
    CONT_T<RID_TUPLE_T> read_rid_table(const char* filepath)
{
    using rid_tuple_t = RID_TUPLE_T;
    using rid_table_t = CONT_T<RID_TUPLE_T>;

    // Open a file stream
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };

    rid_table_t table; // rid table

    // Read table
    {
        rid_tuple_t tuple;
        while (!ifs.eof())
        {
            ifs.read(reinterpret_cast<char*>(&tuple), sizeof(rid_tuple_t));
            if (ifs.gcount() > 0)
                table.push_back(tuple);
        }
#ifndef NDEBUG 
        printf("Finished read RID Table from file %s. number of tuples: %llu\n",
               filepath,
               table.size());
#endif // !NDEBUG
    }

    return table;
}

} // !nameaspace gstream

#endif // !_GSTREAM_DATATYPE_PAGEDB_H_
