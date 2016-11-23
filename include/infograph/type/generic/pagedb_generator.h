#ifndef _INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_
#define _INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_
#include <vector>
#include <memory>

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

template <class PAGE_T>
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

    //explicit db_generator(const char* out_path,  )
}; 

} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_GENERIC_PAGEDB_GENERATOR_H_