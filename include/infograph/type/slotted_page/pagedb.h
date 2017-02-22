#ifndef _INFOGRAPH_PAGEDB_H_
#define _INFOGRAPH_PAGEDB_H_

/* ---------------------------------------------------------------
**
** InfoGraph - InfoLab Graph Library
**
** pagedb.h
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

#include <infograph/type/slotted_page.h>
#include <vector>
#include <fstream>
#include <iterator>

namespace igraph {

/*
* @ Slotted Page Database Information File Representation
*
* SBS: StringBufferSize: 256
*
* 0 ~ 0 + SBS    : Database name
* # ~ # + 8      : The number of pages in database
*
*/

//class pagedb_info
//{
//public:
//    // constructor: using a default ctor
//    pagedb_info() = default;
//    // constructor: read an information file when pagedb_info creation
//    explicit pagedb_info(const char* filepath);
//    // destructor: using a default dtor
//    ~pagedb_info() = default;
//    // read an information file
//    void read_from_file(const char* filepath);
//    // read from ifstream
//    void read_from_stream(std::ifstream& ifs);
//    // write to ofstream
//    void write_to_stream(std::ofstream& ofs);
//
//    static constexpr unsigned int StringBufferSize = 260;
//    char name[StringBufferSize];
//    uint64_t num_pages = 0;
//};
//
//template <typename PAGE_T,
//    template <typename ELEM_T,
//    typename = std::allocator<ELEM_T> >
//    class CONT_T = std::vector >
//    class pagedb
//{
//public:
//    using page_t = PAGE_T;
//    using cont_t = CONT_T<page_t>;
//
//    // constructor: using a default ctor
//    pagedb() = default;
//    // destructor: using a default dtor
//    ~pagedb() = default;
//    // read pagedb files
//    void read(const char* filepath, const size_t bundle_of_pages = 64);
//
//    inline const pagedb_info& info() const;
//
//protected:
//    pagedb_info _info;
//    cont_t _pages;
//};
//
//template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
//void pagedb<PAGE_T, CONT_T>::read(const char* filepath, const size_t bundle_of_pages/* = 64*/)
//{
//    // Open a file stream
//    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };
//
//    // Read a meta data
//    _info.read_from_stream(ifs);
//
//    // Read pages
//    {
//        const size_t chunk_size = sizeof(page_t) * bundle_of_pages;
//        std::vector<page_t> buffer;
//        buffer.resize(chunk_size);
//        uint64_t counter = 0;
//        while (true)
//        {
//            ++counter;
//            ifs.read(buffer.data(), chunk_size);
//            size_t extracted = ifs.gcount();
//            if (extracted == bundle_of_pages)
//            {
//                std::copy(buffer.begin(), buffer.end(), std::back_inserter(_pages));
//            }
//            else
//            {
//                std::copy(buffer.begin(), buffer.begin() + extracted, std::back_inserter(_pages));
//                break;
//            }
//        }
//#ifndef NDEBUG
//        printf("Finished read %llu pages from file. bundle size = %llu, total iteration: %llu\n",
//               _info.num_pages,
//               bundle_of_pages,
//               counter);
//#endif // !NDEBUG
//    }
//}
//
//template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
//const pagedb_info& pagedb<PAGE_T, CONT_T>::info() const
//{
//    return _info;
//}

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

} // !nameaspace igraph

#endif // !_INFOGRAPH_PAGEDB_H_
