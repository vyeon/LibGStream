#ifndef _INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_
#define _INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_
#include <vector>
#include <memory>
#include "pagedb.h"

namespace igraph {

template <typename PAGE_T, 
          template <typename ELEM_T,
                    typename = std::allocator<ELEM_T> >
                    class CONT_T = std::vector >
class rid_table_generator
{
public:
    using page_t = PAGE_T;
    using vertex_id_t = typename page_t::vertex_id_t;
    using adj_page_id_t = typename page_t::adj_page_id_t;
    using adj_offset_t = typename page_t::adj_offset_t;
    using adj_list_size_t = typename page_t::adj_list_size_t;
    using large_page_count_t = typename page_t::large_page_count_t;
    using payload_t = typename page_t::payload_t;
    static constexpr size_t page_size = page_t::page_size;
    using rid_tuple_t = typename page_t::rid_tuple_t;
    
    using cont_t = CONT_T<rid_tuple_t>;
    using rid_table_t = cont_t;

    void iteration_per_vertex(vertex_id_t vid, adj_list_size_t num_edges);
    void flush();
    uint64_t number_of_pages() const;

protected:
    void small_page_iteration(adj_list_size_t num_edges);
    void large_page_iteration(adj_list_size_t num_edges);
    void issue_small_page();
    void issue_lead_page(size_t num_related);
    void issue_ext_pages(size_t num_ext_pages);

    vertex_id_t last_svid{ 0 };
    uint64_t vid_counter{ 0 };
    uint64_t num_pages{ 0 };
    std::shared_ptr<page_t> page{ std::make_shared<page_t>() };
    rid_table_t table;
};

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::iteration_per_vertex(vertex_id_t vid, adj_list_size_t num_edges)
{
    while (vid > (vid_counter + 1))
    {
        this->small_page_iteration(0);
        ++vid_counter;
    }
    if (num_edges > page_t::MaximumEdgesInLeadPage)
        this->large_page_iteration(num_edges);
    else
        this->small_page_iteration(num_edges);
    ++vid_counter;
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::flush()
{
    if (!page->is_empty())
    {
        issue_small_page();
    }
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
uint64_t rid_table_generator<PAGE_T, CONT_T>::number_of_pages() const
{
    return num_pages;
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::small_page_iteration(adj_list_size_t num_edges)
{
    auto scan_result = page->scan();
    bool& is_slot_available = scan_result.first;
    size_t& capacity = scan_result.second;

    if (!is_slot_available || (capacity < num_edges))
        issue_small_page();

    page->add_slot_unsafe(0);
    auto offset = page->number_of_slots() - 1;
    page->add_adj_elems_for_small_page_unsafe(offset, num_edges, nullptr);
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::large_page_iteration(adj_list_size_t num_edges)
{
    if (!page->is_empty())
        issue_small_page();

    size_t required_ext_pages = static_cast<size_t>(std::floor((num_edges - page_t::MaximumEdgesInLeadPage) / page_t::MaximumEdgesInExtPage)) + 1;
    issue_lead_page(required_ext_pages);
    issue_ext_pages(required_ext_pages);
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::issue_small_page()
{
    rid_tuple_t tuple;
    tuple.start_vid = last_svid;
    tuple.num_related_pages = 0; // small page: 0
    table.push_back(tuple);
    last_svid = vid_counter;
    page->clear();
    ++num_pages;
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::issue_lead_page(size_t num_related)
{
    rid_tuple_t tuple;
    tuple.start_vid = last_svid;
    tuple.num_related_pages = num_related; // lead page: the number of related pages
    table.push_back(tuple);
    // this operation does not update a member variable 'last_vid' 
    page->clear();
    ++num_pages;
}

template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
void rid_table_generator<PAGE_T, CONT_T>::issue_ext_pages(size_t num_ext_pages)
{
    rid_tuple_t tuple;
    tuple.start_vid = last_svid;
    for (size_t i = 1; i <= num_ext_pages; ++i)
    {
        tuple.num_related_pages = i; // ext page: page offset from lead page
        table.push_back(tuple);
    }
    last_svid = vid_counter;
    page->clear();
    ++num_pages;
}

template <typename PAGE_T, typename RID_TABLE_T> 
class db_generator
{
public:
    using page_t = PAGE_T;
    using vertex_id_t = typename page_t::vertex_id_t;
    using adj_page_id_t = typename page_t::adj_page_id_t;
    using adj_offset_t = typename page_t::adj_offset_t;
    using adj_list_size_t = typename page_t::adj_list_size_t;
    //using adj_element_t = typename page_t::adj_element_t;
    using large_page_count_t = typename page_t::large_page_count_t;
    using payload_t = typename page_t::payload_t;
    static constexpr size_t page_size = page_t::page_size;
    using rid_tuple_t = typename page_t::rid_tuple_t;
    using rid_table_t = RID_TABLE_T;
    using edge_payload_t = typename page_t::adj_payload_t;

protected:
    template <typename>
    struct edge_template
    {
        vertex_id_t src;
        vertex_id_t dst;
        payload_t payload;
        void to_adj_elem(typename page_t::adj_elem_t* dst)
        {
            dst->adj_page_id = get_page_id(dst);
            dst->adj_offset = get_slot_offset(dst->adj_page_id, dst);
            dst->payload = payload;
        }
    };

    template <>
    // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
    struct edge_template<void>
    {
        vertex_id_t src;
        vertex_id_t dst;
        void to_adj_elem(typename page_t::adj_elem_t* dst)
        {
            dst->adj_page_id = get_page_id(dst);
            dst->adj_offset = get_slot_offset(dst->adj_page_id, dst);
        }
    };

public:
    using edge_t = edge_template<payload_t>;

    explicit db_generator(pagedb_info& info, rid_table_t& rid_table_, std::ofstream& ofs_, size_t bufsize_);
    void iteration_per_vertex(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void flush();

protected:
    void small_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void large_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void issue_page(igraph::page_flag flag);

    void update_list_buffer(edge_t* edges, size_t num_edges);
    void update_list_buffer(typename page_t::adj_element_t* adj_elems, size_t num_edges);
    adj_page_id_t get_page_id(vertex_id_t vid);
    adj_offset_t  get_slot_offset(adj_page_id_t pid, vertex_id_t vid);

    rid_table_t& rid_table;
    std::ofstream& ofs;
    std::shared_ptr<page_t> page{ std::make_shared<page_t>() };
    std::shared_ptr<page_t> page_buffer;
    std::vector<typename page_t::adj_element_t> list_buffer;
    uint64_t page_counter{ 0 };
    uint64_t vid_counter{ 0 };
    uint64_t num_issued{ 0 };
    size_t bufsize{ 0 };
    
    // disable default ctor
    db_generator() = delete;
};

template <typename PAGE_T, typename RID_TABLE_T>
db_generator<PAGE_T, RID_TABLE_T>::db_generator(pagedb_info& info, rid_table_t& rid_table_, std::ofstream& ofs_, size_t bufsize_):
    rid_table{rid_table_},
    ofs{ofs_},
    page_buffer { new page_t[bufsize_], [](page_t* p){ delete[]p; } },
    bufsize {bufsize_}
{
    // Writing the meta data (PageDB Information) to out stream
    info.write_to_stream(ofs);
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::iteration_per_vertex(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges)
{
    while (vid > (vid_counter + 1))
    {
        this->small_page_iteration(++vid_counter, nullptr, 0);
    }
    if (num_edges > page_t::MaximumEdgesInLeadPage)
        this->large_page_iteration(vid, edges, num_edges);
    else
        this->small_page_iteration(vid, edges, num_edges);
    ++vid_counter;
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::flush()
{
    ofs.write(reinterpret_cast<char*>(page_buffer.get()), sizeof(page_t) * page_counter);
    page_counter = 0;
    ++num_issued;
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::small_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges)
{
    auto scan_result = page->scan();
    bool& is_slot_available = scan_result.first;
    size_t& capacity = scan_result.second;

    if (!is_slot_available || (capacity < num_edges))
        issue_page(igraph::SMALL_PAGE);

    page->add_slot_unsafe(vid);

    if (num_edges == 0)
        return;

    auto offset = page->number_of_slots() - 1;
    update_list_buffer(edges, num_edges);
    page->add_adj_elems_for_small_page_unsafe(offset, num_edges, list_buffer.data());
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::large_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges)
{
    if (!page->is_empty())
        issue_page(igraph::SMALL_PAGE);

    size_t required_ext_pages = static_cast<size_t>(std::floor((num_edges - page_t::MaximumEdgesInLeadPage) / page_t::MaximumEdgesInExtPage)) + 1;
    size_t offset = 0;

    // Processing a lead page
    {
        constexpr size_t capacity = page_t::MaximumEdgesInLeadPage;
        page->add_slot_unsafe(vid);
        update_list_buffer(edges, capacity);
        page->add_adj_elems_for_lead_page_unsafe(0, num_edges, capacity, list_buffer.data());
        offset += capacity;
        issue_page(static_cast<igraph::page_flag>(igraph::LARGE_PAGE | igraph::LEAD_PAGE));
    }

    size_t remained_edges = num_edges - page_t::MaximumEdgesInLeadPage;
    // Processing extended pages
    for (size_t i = 0; i < required_ext_pages; ++i)
    {
        constexpr size_t capacity = page_t::MaximumEdgesInExtPage;
        size_t num_edges_per_page = (remained_edges >= capacity) ? capacity : remained_edges;
        page->add_extended_slot_unsafe(vid);
        update_list_buffer(edges + offset, num_edges_per_page);
        page->add_adj_elems_for_ext_page_unsafe(0, num_edges_per_page, list_buffer.data());
        offset += num_edges_per_page;
        remained_edges -= num_edges_per_page;
        issue_page(static_cast<igraph::page_flag>(igraph::LARGE_PAGE | igraph::EXTENDED_PAGE));
    }
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::issue_page(igraph::page_flag flag)
{
    page_t* buffer = page_buffer.get();
    page->set_flags(flag);
    memmove(&buffer[page_counter++], page.get(), sizeof(page_t));
    page->clear();
    if (page_counter == bufsize)
        flush();
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::update_list_buffer(edge_t* edges, size_t num_edges)
{
    list_buffer.clear();
    typename page_t::adj_element_t elem;
    for (size_t i = 0; i < num_edges; ++i)
    {
        edges[i].to_adj_elem(&elem);
        list_buffer.push_back(elem);
    }
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::update_list_buffer(typename PAGE_T::adj_element_t* adj_elems, size_t num_edges)
{
    list_buffer.clear();
    list_buffer.insert(list_buffer.begin(), adj_elems, adj_elems + num_edges);
    /*for (size_t i = 0; i < num_edges; ++i)
    list_buffer.push_back(edges[i]);*/
}

template <typename PAGE_T, typename RID_TABLE_T>
typename db_generator<PAGE_T, RID_TABLE_T>::adj_page_id_t db_generator<PAGE_T, RID_TABLE_T>::get_page_id(vertex_id_t vid)
{
    for (size_t i = 0; i < rid_table.size(); ++i)
    {
        const rid_tuple_t& tuple = rid_table[i];
        if (tuple.start_vid == vid)
            return i;
        if (tuple.start_vid > vid)
            return i - 1;
    }
    // Since the number of vertices is unknown at this point, if the above lookup  phase does not find the corresponding VID, it is assumed to exist on the last page.
    return rid_table.size() - 1;
}

template <typename PAGE_T, typename RID_TABLE_T>
typename db_generator<PAGE_T, RID_TABLE_T>::adj_offset_t db_generator<PAGE_T, RID_TABLE_T>::get_slot_offset(adj_page_id_t pid, vertex_id_t vid)
{
    const rid_tuple_t& tuple = rid_table[pid];
    return vid - tuple.start_vid;
}

} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_