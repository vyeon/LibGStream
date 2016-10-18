/* ---------------------------------------------------------------
**
** InfoGraph - InfoLab Graph Library
**
** slotted_page.h
** Generic C++ implementation of the "Slotted Page" which is a
** data structure for graph-data processing
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

/* ---------------------------------------------------------------
**
** Slotted page represntation (Page size: user defined)
** +-------------------------------------------------------------+
** |                                                             |
** |                                                             |
** |                      Data Section                           |
** |               size = page size - footer size       +--------+
** |                                                    | footer |
** +----------------------------------------------------+--------+
**
** Detailed slotted page representation
** +-------------------------------------------------------------+
** | S0 adj-list size | S0 adj-elem #0 | S0 adj-elem #1 | S0 adj-|
** +-------------------------------------------------------------+
** |elem #2 | S0 adj-elem #3 | ...                               |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                ... | S0 adj-elem #N | S1 adj-list size | S1 |
** +-------------------------------------------------------------+
** | adj-elem #0 | S2 adj-elem #1 | S1 adj-elem #2 | S1 adj-elem |
** +-------------------------------------------------------------+
** | #3 | ...                                                    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                                    ...| S1 adj-elem #M |    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                 | slot #1 (S1) | slot #0 (S0) | page footer |
** +-------------------------------------------------------------+
**
** Adjacency list size: user defined type
** +---------------------------------------------+
** | Slot #@ adjacency list size (user-def type) |
** +---------------------------------------------+
**
** Adjacency element representation
** +------------------------------------------------------------------------------------+
** | adj_page_id (user-def type) | adj_offset (user-def type) | payload (user-def type) |
** +------------------------------------------------------------------------------------+
**
** Slot representation
** +--------------------------------------------------------+
** | vertex_id (user-def type) | internal_offset (uint32_t) |
** +--------------------------------------------------------+
**
** Page footer representation: 16bytes
** +-------------------------------------------------------------------------+
** | padding (4byte) | flags (uint32_t) | front (uint32_t) | rear (uint32_t) |
** +-------------------------------------------------------------------------+
**
** ------------------------------------------------------------ */

#ifndef _INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_H_
#define _INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_H_

#include <infograph/mpl.h>
#include <memory>

namespace igraph {

const size_t SIZE_1KB = 1024;
const size_t SIZE_1MB = SIZE_1KB * 1024;
const size_t SIZE_1GB = SIZE_1MB * 1024;
const size_t SIZE_2GB = SIZE_1GB * 2;
const size_t SIZE_3GB = SIZE_1GB * 3;
const size_t SIZE_4GB = SIZE_1GB * 4;
const size_t SIZE_5GB = SIZE_1GB * 5;
const size_t SIZE_6GB = SIZE_1GB * 6;

#define SLOTTED_PAGE_TEMPLATE \
template <\
typename __vertex_id_t,\
typename __adj_page_id_t,\
typename __adj_offset_t,\
typename __adj_list_size_t,\
size_t   PAGE_SIZE,\
typename __adj_payload_t,\
typename __offset_t,\
typename __internal_index_t\
>
#define SLOTTED_PAGE_TEMPLATE_ARGS __vertex_id_t, __adj_page_id_t, __adj_offset_t, __adj_list_size_t, PAGE_SIZE, __adj_payload_t, __offset_t, __internal_index_t

enum page_flag
{
    _BASE = 0x0001,
    LEAD_PAGE = _BASE,
    EXTENDED_PAGE = _BASE << 1,
    SMALL_PAGE = _BASE << 2,
    LARGE_PAGE = _BASE << 3
};

#pragma pack (push, 1)
template <
    typename __vertex_id_t,
    typename __adj_page_id_t,   // adjacency ID represents an ID of target slotted page
    typename __adj_offset_t,    // adjacency offset represents an offset(index) of a slot which located at corresponding page
    typename __adj_list_size_t, // adjacency list size represents a number of edges each vertex have 
    size_t   PAGE_SIZE = SIZE_1MB,
    typename __adj_payload_t = void,
    typename __offset_t = uint64_t,
    typename __internal_index_t = uint32_t 
>
class slotted_page
{
    /* Constraint 1. Page size must smaller than 4GB */
    static_assert((PAGE_SIZE <= SIZE_4GB), "Generic Slotted Page: Constraint1. Page size must smaller than 4GB!");
    /* Constraint 2. Payload type must be a Plain old data (POD) type */
    static_assert((std::is_void<__adj_payload_t>::value || std::is_pod<__adj_payload_t>::value), "Generic Slotted Page: Constraint2. Payload type must be a Plain old data (POD) type");
    
/* Typedefs and Constant definitions */
public:
    using type = slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>;
    using shared_ptr = std::shared_ptr<type>;
    using unique_ptr = std::unique_ptr<type>;
    using weak_ptr = std::weak_ptr<type>;
    using adj_payload_t   = __adj_payload_t;
    using inter_idx_t = __internal_index_t;
    using vertex_id_t = __vertex_id_t;
    using adj_page_id_t = __adj_page_id_t;
    using adj_offset_t = __adj_offset_t;
    using adj_list_size_t = __adj_list_size_t;
    using offset_t = __offset_t; 

    static constexpr size_t adj_payload_size = mpl::tricky_sizeof<adj_payload_t>::value;

    template <typename PayloadTy>
    struct adj_element_t_base
    {
        adj_page_id_t adj_page_id;
        adj_offset_t  adj_offset;
        PayloadTy     payload;
    };

    template <>
    // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
    struct adj_element_t_base<void>
    {
        adj_page_id_t adj_page_id;
        adj_offset_t  adj_offset;
    };

    using adj_element_t = adj_element_t_base<adj_payload_t>;

    struct slot_t
    {
        vertex_id_t vertex_id;
        inter_idx_t internal_offset;
    };

    struct footer_t
    {
        uint32_t    padding;
        uint32_t    flags;
        inter_idx_t front;
        inter_idx_t rear;
    };

    struct rid_tuple_t
    {
        vertex_id_t start_vid; // vertex_id_t
        uint32_t num_related_pages;
    };

    static constexpr size_t DATA_SECTION_SIZE = PAGE_SIZE - sizeof(footer_t);

    static constexpr size_t MaximumEdgesInLeadPage = (DATA_SECTION_SIZE - sizeof(slot_t) - sizeof(adj_list_size_t)) / sizeof(adj_element_t);
    static constexpr size_t MaximumEdgesInExtPage = (DATA_SECTION_SIZE - sizeof(slot_t)) / sizeof(adj_element_t);

/* Member functions */
public:
    // Default constructor
    slotted_page() = default;
    // Initialize with specific page flag
    explicit slotted_page(page_flag flag): slotted_page()
    {
        footer.flags = flag;
    }
    slotted_page(const type& other)
    {
        memmove(this, &other, PAGE_SIZE);
    }
    slotted_page(type&& other)
    {
        memmove(this, &other, PAGE_SIZE);
    }
    type& operator=(const type& other)
    {
        memmove(this, &other, PAGE_SIZE);
        return *this;
    }
    type& operator=(type&& other)
    {
        memmove(this, &other, PAGE_SIZE);
        return *this;
    }
    // Destructor
    ~slotted_page() noexcept = default;
    // Scan operation: scans free space in the slotted page, and returns result of a couple of informations as follows
    // (1) Whether it is possible to store new slot to this page or not (boolean)
    // (2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)
    std::pair<bool /* (1) */, size_t /* (2) */> scan() const;
    // Scan operation for extended page: scannnig the page without regard to adjacency list size of slot
    // This operation will returns,
    // (1) Whether it is possible to store new slot to this page or not (boolean)
    //     Note: If the page is not empty, this operation always failure!
    // (2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)
    std::pair<bool /* (1) */, size_t /* (2) */> scan_for_extended_page() const;
    // GEt a flag of page
    inline decltype(footer_t::flags)& flags()
    {
        return footer.flags;
    }
    // Get an address of data section in a slotted page
    inline uint8_t*& data() noexcept
    {
        return data_section;
    }
    // Get a number of slots stored in a slotted page
    inline size_t number_of_slots() const noexcept
    {
        //assert(((DATA_SECTION_SIZE - footer.rear) % sizeof(slot_t)) == 0);
        return (DATA_SECTION_SIZE - footer.rear) / sizeof(slot_t);
    }
    // Get a pointer of slot via slot offset(index) as slot ID
    inline slot_t* slot_ptr(offset_t offset) noexcept
    {
        return reinterpret_cast<slot_t*>(&data_section[DATA_SECTION_SIZE - (sizeof(slot_t) * (offset + 1))]);
    }
    // Get a reference of slot via slot offset(index) as slot ID
    inline slot_t& slot_ref(offset_t offset) const noexcept
    {
        return *const_cast<slot_t*>(reinterpret_cast<const slot_t*>(&data_section[DATA_SECTION_SIZE - (sizeof(slot_t) * (offset + 1))]));
    }
    // Get a slot via slot offset(index) as slot ID
    inline slot_t get_slot(offset_t offset) const noexcept
    {
        return *reinterpret_cast<const slot_t*>(&data_section[DATA_SECTION_SIZE - (sizeof(slot_t) * (offset + 1))]);
    }
    // Get a pointer of adjacency list via slot offset(index) as slot ID
    inline adj_element_t* adj_list(offset_t offset) noexcept
    {
        slot_t& slot = slot_ref(offset);
        if (footer.flags & page_flag::EXTENDED_PAGE)
            return reinterpret_cast<adj_element_t*>(&data_section[static_cast<offset_t>(slot.internal_offset)]);
        else
            return reinterpret_cast<adj_element_t*>(&data_section[static_cast<offset_t>(slot.internal_offset) + sizeof(adj_list_size_t)]);
    }
    // Get a size of adjacency list via slot offset(index) as slot ID
    inline adj_list_size_t adj_list_size(offset_t offset) const noexcept
    {
        slot_t& slot = slot_ref(offset);
        return *reinterpret_cast<const adj_list_size_t*>(&data_section[static_cast<offset_t>(slot.internal_offset)]);
    }
    // Get the number of elements of adjacency list in the page via slot offset(index) as slot ID
    inline adj_list_size_t local_adj_list_size(offset_t offset) const noexcept
    {
        if (footer.flags & page_flag::SMALL_PAGE)
        {
            return adj_list_size(offset);
        }
        
        if (footer.flags & page_flag::LEAD_PAGE)
        {
            adj_list_size_t size = (footer.front - sizeof(adj_list_size_t)) / sizeof(adj_element_t);
            return size;
        }
        
        if (footer.flags & page_flag::EXTENDED_PAGE)
        {
            adj_list_size_t size = footer.front / sizeof(adj_element_t);
            return size;
        }
        
        // INVALID TYPE
        return 0;
    }
    inline bool is_empty() const noexcept
    {
        if (footer.front == 0)
            return true;
        else
            return false;
    }
    inline void clear() noexcept
    {
        memset(data_section, 0, DATA_SECTION_SIZE);
        footer.padding = 0;
        footer.front = 0;
        footer.rear = DATA_SECTION_SIZE;
    }
    inline void set_flags(page_flag& flags)
    {
        footer.flags = flags;
    }
    // Add slot operation: add a slot into slotted page, this operation returns,
    // (1) Operation result (boolean)
    // (2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)
    std::pair<bool /* (1) */, size_t /* (2) */> add_slot(vertex_id_t vid) noexcept;
    // Unsafe version of add slot operation: add a slot into slotted page
    // This operation does not checking and handling errors
    // returns the number of free space (byte)
    size_t add_slot_unsafe(vertex_id_t vid);
    // Add extened slot operation: add a slot into slotted page without adjacency list size for "Large-page" generation
    // If a page is not empty, this operation will fail! 
    // This operation returns as same as add_slot() operation
    // (1) Operation result (boolean)
    // (2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)
    std::pair<bool /* (1) */, size_t /* (2) */> add_extened_slot(vertex_id_t vid);
    // Unsafe version of add extended slot operation for "Large-page" generation
    size_t add_extended_slot_unsafe(vertex_id_t vid);
    // @ Operation for the small page
    // Add the elements of adjacency list to specific slot
    // This operation is unsafe, require attention to use
    void add_adj_elems_for_small_page_unsafe(offset_t slot_offset, adj_list_size_t list_size, adj_element_t* elements);
    // @ Operation for the lead page
    // Add the elements of adjacency list to specific slot
    // This operation is unsafe, require attention to use
    void add_adj_elems_for_lead_page_unsafe(offset_t slot_offset, adj_list_size_t list_size, adj_list_size_t num_elems_in_page, adj_element_t* elements);
    // @ Operation for the extended page
    // Add the elements of adjacency list to specific slot
    // This operation is unsafe, require attention to use
    void add_adj_elems_for_ext_page_unsafe(offset_t slot_offset, adj_list_size_t list_size, adj_element_t* elements);

    /* Utility functions */
    static size_t storable_list_size();
    static size_t storable_extended_list_size();
    /* Member variables */
public:
    uint8_t  data_section[DATA_SECTION_SIZE];
    footer_t footer{ 0, 0, 0, DATA_SECTION_SIZE };
};

SLOTTED_PAGE_TEMPLATE
std::pair<bool, size_t> slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::scan() const
{
    const size_t free_space = footer.rear - footer.front;
    if (free_space < (sizeof(slot_t) + sizeof(adj_list_size_t)))
        return std::make_pair(false, 0);
    size_t free_space_for_elems = (free_space - sizeof(slot_t) - sizeof(adj_list_size_t));
    return std::make_pair(true, static_cast<size_t>(free_space_for_elems / sizeof(adj_element_t)));
}

SLOTTED_PAGE_TEMPLATE
std::pair<bool, size_t> slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::scan_for_extended_page() const
{
    if (!is_empty())
    return std::make_pair(false, 0);
    const size_t free_space = footer.rear - footer.front;
    if (free_space < (sizeof(slot_t)))
    return std::make_pair(false, 0);
    size_t free_space_for_elems = (free_space - sizeof(slot_t));
    return std::make_pair(true, static_cast<size_t>(free_space_for_elems / sizeof(adj_element_t)));
}

SLOTTED_PAGE_TEMPLATE
std::pair<bool, size_t> slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_slot(vertex_id_t vid) noexcept
{
    auto scan_result = scan();
    if (false == scan_result.first)
        return std::make_pair(false, 0);
    size_t free_space_for_elems = add_slot_unsafe(vid);
    return std::make_pair(true, static_cast<size_t>(free_space_for_elems / sizeof(adj_element_t)));
}

SLOTTED_PAGE_TEMPLATE
size_t slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_slot_unsafe(vertex_id_t vid)
{
    slot_t new_slot{ vid, footer.front };
    footer.front += sizeof(adj_list_size_t);
    footer.rear -= sizeof(slot_t);
    memmove(&data_section[footer.rear], &new_slot, sizeof(slot_t));
    return footer.rear - footer.front;
}

SLOTTED_PAGE_TEMPLATE
std::pair<bool, size_t> slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_extened_slot(vertex_id_t vid)
{
    if (!is_empty())
    return std::make_pair(false, 0);
    auto scan_result = scan();
    if (false == scan_result.first)
    return std::make_pair(false, 0);
    size_t free_space_for_elems = add_extended_slot_unsafe(vid);
    return std::make_pair(true, static_cast<size_t>(free_space_for_elems / sizeof(adj_element_t)));
}

SLOTTED_PAGE_TEMPLATE
size_t slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_extended_slot_unsafe(vertex_id_t vid)
{
    slot_t new_slot{ vid, footer.front };
    footer.rear -= sizeof(slot_t);
    memmove(&data_section[footer.rear], &new_slot, sizeof(slot_t));
    return footer.rear - footer.front;
}

SLOTTED_PAGE_TEMPLATE
void slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_adj_elems_for_small_page_unsafe(offset_t slot_offset, adj_list_size_t list_size, adj_element_t* elements)
{
    slot_t& slot = slot_ref(slot_offset);
    adj_list_size_t* size_section = reinterpret_cast<adj_list_size_t*>(&data_section[slot.internal_offset]);
    // Copying the size of adjacency list
    *size_section = list_size;

    if (nullptr != elements)
    {
        // Copying the adjacency elements
        adj_element_t*   elem_section = reinterpret_cast<adj_element_t*>(&data_section[slot.internal_offset + sizeof(adj_list_size_t)]);
        memmove(elem_section, elements, sizeof(adj_element_t) * list_size);
    }
    
    footer.front += sizeof(adj_element_t) * list_size;
}

SLOTTED_PAGE_TEMPLATE
void slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_adj_elems_for_lead_page_unsafe(offset_t slot_offset, adj_list_size_t list_size, adj_list_size_t num_elems_in_page, adj_element_t* elements)
{
    slot_t& slot = slot_ref(slot_offset);
    adj_list_size_t* size_section = reinterpret_cast<adj_list_size_t*>(&data_section[slot.internal_offset]);
    // Copying the size of adjacency list
    *size_section = list_size;

    // Copying the adjacency elements
    if (nullptr != elements)
    {
        adj_element_t*   elem_section = reinterpret_cast<adj_element_t*>(&data_section[slot.internal_offset + sizeof(adj_list_size_t)]);
        memmove(elem_section, elements, sizeof(adj_element_t) * num_elems_in_page);
    }
    footer.front += sizeof(adj_element_t) * num_elems_in_page;
}

SLOTTED_PAGE_TEMPLATE
void slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::add_adj_elems_for_ext_page_unsafe(offset_t slot_offset, adj_list_size_t num_elems_in_page, adj_element_t* elements)
{
    slot_t& slot = slot_ref(slot_offset);

    if (nullptr != elements)
    {
        adj_element_t*   elem_section = reinterpret_cast<adj_element_t*>(&data_section[slot.internal_offset]);
        // Copying the adjacency elements
        memmove(elem_section, elements, sizeof(adj_element_t) * num_elems_in_page);
        footer.front += sizeof(adj_element_t) * num_elems_in_page;
    }
}

SLOTTED_PAGE_TEMPLATE
size_t slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::storable_list_size()
{
    size_t free_space_for_elems = (DATA_SECTION_SIZE - sizeof(slot_t) - sizeof(adj_list_size_t));
    return (free_space_for_elems / sizeof(adj_element_t));
}

SLOTTED_PAGE_TEMPLATE
size_t slotted_page<SLOTTED_PAGE_TEMPLATE_ARGS>::storable_extended_list_size()
{
    size_t free_space_for_elems = (DATA_SECTION_SIZE - sizeof(slot_t));
    return (free_space_for_elems / sizeof(adj_element_t));
}

#pragma pack(pop)

#undef SLOTTED_PAGE_TEMPLATE
#undef SLOTTED_PAGE_TEMPLATE_ARGS

} // !namespace igraph

#include <fstream>
#include <vector>
#include <cassert>

namespace igraph {

template <class PAGE_T>
void pages_to_file(const char* path, PAGE_T* pages, const size_t number_of_pages, const size_t bundle_of_pages = 64)
{
    std::ofstream ofs{ path, std::ios::out | std::ios::binary };
    uint64_t remained_pages = number_of_pages;
    size_t offset = 0;
    size_t counter = 0;
    char* data = reinterpret_cast<char*>(pages);
    ofs.write(reinterpret_cast<char*>(&remained_pages), sizeof(uint64_t));
    while (remained_pages > 0)
    {
        size_t num_pages = (remained_pages >= bundle_of_pages) ? bundle_of_pages : remained_pages;
        size_t bytes_requested = sizeof(PAGE_T) * num_pages;
        ofs.write(data + offset, bytes_requested);

        remained_pages -= num_pages;
        offset += bytes_requested;
        ++counter;
    }
#ifndef NDEBUG
    printf("Finished writing %llu pages to file. bundle size = %llu, total iteration: %llu\n",
           number_of_pages,
           bundle_of_pages,
           counter);
#endif
}

template <class PAGE_T>
std::vector<PAGE_T> pages_from_file(const char* path, const size_t bundle_of_pages = 64)
{
    std::ifstream ifs{ path, std::ios::in | std::ios::binary };
    const size_t chunk_size = sizeof(PAGE_T) * bundle_of_pages;
    uint64_t number_of_pages = 0;
    ifs.read(reinterpret_cast<char*>(&number_of_pages), sizeof(uint64_t));
    std::vector<PAGE_T> pages;
    pages.resize(number_of_pages);
    size_t offset = 0;
    size_t counter = 0;
    while (true)
    {
        ++counter;
        ifs.read(reinterpret_cast<char*>(pages.data()) + offset, chunk_size);
        size_t extracted = ifs.gcount();
        assert(extracted % sizeof(PAGE_T) == 0);
        offset += extracted;
        if (extracted != chunk_size)
            break;
    }
#ifndef NDEBUG
    printf("Finished writing %llu pages to file. bundle size = %llu, total iteration: %llu\n",
           number_of_pages,
           bundle_of_pages,
           counter);
#endif
    return pages;
}

template <class PAGE_T>
PAGE_T* get_page_ptr_from_vid(std::vector<PAGE_T>& pages, std::vector<typename PAGE_T::rid_tuple_t>& rid_tuples, typename PAGE_T::vertex_id_t vid)
{
    for (size_t pid = 0; pid < rid_tuples.size(); ++pid)
    {
        if (rid_tuples[pid].vertex_id == vid)
        {
            return &pages[pid];
        }
        else if (rid_tuples[pid].vertex_id > vid)
        {
            return &pages[pid - 1];
        }
    }
    PAGE_T& last_page = pages[pages.size() - 1];
    size_t number_of_slots_in_last_page = last_page.number_of_slots();
    if (vid <= (last_page.slot_ref(0).vertex_id + (number_of_slots_in_last_page)))
        return &last_page;
    return nullptr; // NOT FOUND
}

template <class PAGE_T>
typename PAGE_T::slot_t* get_slot_ptr_from_vid(PAGE_T& page, typename PAGE_T::vertex_id_t vid)
{
    const size_t number_of_slots = page.number_of_slots();
    for (size_t i = 0; i < number_of_slots; ++i)
    {
        auto slot = page.slot_ref(i);
        if (slot.vertex_id == vid)
            return &slot;
    }
    return nullptr; // NOT FOUND
}

template <class PAGE_T>
std::pair<bool /* success or failure */, typename PAGE_T::offset_t /* slot offset */>
get_slot_offset_from_vid(PAGE_T& page, typename PAGE_T::vertex_id_t vid)
{
    const size_t number_of_slots = page.number_of_slots();
    for (typename PAGE_T::offset_t i = 0; i < number_of_slots; ++i)
    {
        auto slot = page.slot_ref(i);
        if (slot.vertex_id == vid)
            return std::make_pair(true, i);
    }
    return std::make_pair(false, 0); // NOT FOUND
}

} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_H_