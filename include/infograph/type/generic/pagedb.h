#ifndef _INFOGRAPH_TYPE_GENERIC_PAGEDB_H_
#define _INFOGRAPH_TYPE_GENERIC_PAGEDB_H_

#include <infograph/type/generic/slotted_page.h>
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
    void read_from_file(const char* filepath);
    
    static constexpr unsigned int StringBufferSize = 260;
    char name[StringBufferSize];
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
    using cont_t = CONT_T<page_t>;

    // constructor: using a default ctor
    pagedb() = default;
    // destructor: using a default dtor
    ~pagedb() = default;
    // read pagedb files
    void read(const char* dbpath, const size_t bundle_of_pages = 64);

    inline const pagedb_info& info() const;

protected:
    pagedb_info _info;
    cont_t _pages;
};

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void pagedb<PAGE_T, CONT_T>::read(const char* dbpath, const size_t bundle_of_pages/* = 64*/)
{ 
    // Read an information file
    {
        char path_buffer[_MAX_PATH] = { 0, };
        sprintf_s(path_buffer, "%s%s", dbpath, "\\dbinfo");
        _info.read_from_file(path_buffer);
    }

    // Read pages
    {
        std::ifstream ifs{ std::string{dbpath} +"\\pages", std::ios::in | std::ios::binary };
        const size_t chunk_size = sizeof(page_t) * bundle_of_pages;
        std::vector<page_t> buffer;
        buffer.resize(chunk_size);
        uint64_t counter = 0;
        while(true)
        {
            ++counter;
            ifs.read(buffer.data(), chunk_size);
            size_t extracted = ifs.gcount();
            if (extracted == bundle_of_pages)
            {
                std::copy(buffer.begin(), buffer.end(), std::back_inserter(_pages));
            }
            else
            {
                std::copy(buffer.begin(), buffer.begin() + extracted, std::back_inserter(_pages));
                break;
            }
        }
#ifndef NDEBUG
        printf("Finished read %llu pages from file. bundle size = %llu, total iteration: %llu\n",
               _info.num_pages,
               bundle_of_pages,
               counter);
#endif
    }
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
const pagedb_info& pagedb<PAGE_T, CONT_T>::info() const
{
    return _info;
}

} // !namespace igraph

#endif // !_INFOGRAPH_PAGEDB_H_