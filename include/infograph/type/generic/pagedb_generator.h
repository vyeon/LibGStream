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
    
    using cont_t = CONT_T<page_t>;
    using rid_table_t = cont_t;

    void iteration_per_vertex(vertex_id_t vid, adj_list_size_t num_edges);
    void flush();

protected:
    void small_page_iteration(adj_list_size_t num_edges);
    void large_page_iteration(adj_list_size_t num_edges);
    void issue_small_page();
    void issue_lead_page(size_t num_related);
    void issue_ext_pages(size_t num_ext_pages);

    vertex_id_t last_svid{ 0 };
    size_t vid_counter{ 0 };
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
    using large_page_count_t = typename page_t::large_page_count_t;
    using payload_t = typename page_t::payload_t;
    static constexpr size_t page_size = page_t::page_size;
    using rid_tuple_t = typename page_t::rid_tuple_t;
    using rid_table_t = RID_TABLE_T;
    using edge_payload_t = typename page_t::adj_payload_t;

protected:
    template <typename payload_t>
    struct edge_template
    {
        vertex_id_t src;
        vertex_id_t dst;
        payload_t   payload;
    };

    template <>
    // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
    struct edge_template<void>
    {
        vertex_id_t src;
        vertex_id_t dst;
    };

public:
    using edge_t = edge_template<edge_payload_t>;

    explicit db_generator(pagedb_info& info_, rid_table_t& rid_table_, std::ofstream& ofs_, size_t bufsize);
    void iteration_per_vertex(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void flush();

protected:
    void small_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void large_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges);
    void issue_page(igraph::page_flag flag);

    void update_list_buffer(edge_t* edges, size_t num_edges);
    adj_page_id_t get_page_id(vertex_id_t vid);
    adj_offset_t  get_slot_offset(adj_page_id_t pid, vertex_id_t vid);

    rid_table_t& rid_table;
    std::ofstream& ofs;
    std::shared_ptr<page_t> page{ std::make_shared<page_t>() };
    std::shared_ptr<page_t> page_buffer;
    std::vector<typename page_t::adj_element_t> list_buffer;
    size_t page_counter{ 0 };
    size_t vid_counter{ 0 };
    size_t num_issued{ 0 };
    
    // disable default ctor
    db_generator() = delete;
};

template <typename PAGE_T, typename RID_TABLE_T>
db_generator<PAGE_T, RID_TABLE_T>::db_generator(pagedb_info& info_, rid_table_t& rid_table_, std::ofstream& ofs_, size_t bufsize):
    rid_table{rid_table_},
    ofs{ofs_},
    page_buffer { new page_t[bufsize], [](page_t* p){ delete[]p; } }
{
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
    std::ofstream outfile_info{ dbname + ".info", std::ios::out | std::ios::binary };
    uint64_t num_pages = num_issued;
    outfile_info.write(reinterpret_cast<char*>(&num_pages), sizeof(decltype(num_pages)));
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::large_page_iteration(vertex_id_t vid, edge_t* edges, adj_list_size_t num_edges)
{

}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::issue_page(igraph::page_flag flag)
{
}

template <typename PAGE_T, typename RID_TABLE_T>
void db_generator<PAGE_T, RID_TABLE_T>::update_list_buffer(edge_t* edges, size_t num_edges)
{
}

template <typename PAGE_T, typename RID_TABLE_T>
typename db_generator<PAGE_T, RID_TABLE_T>::adj_page_id_t db_generator<PAGE_T, RID_TABLE_T>::get_page_id(vertex_id_t vid)
{
}

template <typename PAGE_T, typename RID_TABLE_T>
typename db_generator<PAGE_T, RID_TABLE_T>::adj_offset_t db_generator<PAGE_T, RID_TABLE_T>::get_slot_offset(adj_page_id_t pid, vertex_id_t vid)
{
}
} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_