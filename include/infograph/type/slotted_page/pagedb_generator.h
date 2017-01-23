#ifndef _INFOGRAPH_PAGEDB_GEN_H_
#define _INFOGRAPH_PAGEDB_GEN_H_

/* ---------------------------------------------------------------
**
** InfoGraph - InfoLab Graph Library
**
** pagedb_generator.h
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

#include <infograph/type/slotted_page/pagedb.h>
#include <vector>

namespace igraph {

enum class generator_error_t
{
    success,
    init_failed_empty_edgeset,
};

template <typename PageBuilderTy,
    template <typename ELEM_T,
    typename = std::allocator<ELEM_T> >
    class CONT_T = std::vector >
    class rid_table_generator
{
public:
    using builder_t = PageBuilderTy;
    ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(builder_t);
    ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(builder_t);
    using rid_tuple_t = rid_tuple_template<vertex_id_t, ___size_t>;
    using cont_t = CONT_T<rid_tuple_t>;
    using rid_table_t = cont_t;
    using edge_t = edge_template<vertex_id_t, edge_payload_t>;

    using edgeset_t = std::vector<edge_t>;
    using iteration_result = std::pair<edgeset_t /* sorted vertex #'s edgeset */, vertex_id_t /* max_vid */>;
    using edge_iterator_t = std::function< iteration_result() >;
    std::pair<generator_error_t /* error info */, rid_table_t /* result table */> generate(edge_iterator_t edge_iterator);
    rid_table_t generate(edge_t* sorted_edges, ___size_t num_edges);

protected:
    void init();
    void iteration_per_vertex(rid_table_t& out_table, ___size_t num_edges);
    void flush(rid_table_t& table);
    void small_page_iteration(rid_table_t& table, ___size_t num_edges);
    void large_page_iteration(rid_table_t& table, ___size_t num_edges);
    void issue_sp(rid_table_t& table);
    void issue_lp_head(rid_table_t& table, ___size_t num_related);
    void issue_lp_exts(rid_table_t& table, ___size_t num_ext_pages); 

    vertex_id_t next_svid;
    vertex_id_t vid_counter;
    ___size_t  num_pages;
    std::shared_ptr<builder_t> page{ std::make_shared<builder_t>() };
};

#define RID_TABLE_GENERATOR_TEMPLATE template <typename PAGE_T, template <typename ELEM_T, typename> class CONT_T>
#define RID_TABLE_GENERATOR rid_table_generator<PAGE_T, CONT_T>

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::init()
{
    next_svid = 0;
    vid_counter = 0;
    num_pages = 0;
}

RID_TABLE_GENERATOR_TEMPLATE
std::pair<generator_error_t, typename RID_TABLE_GENERATOR::rid_table_t> RID_TABLE_GENERATOR::generate(edge_iterator_t iterator)
{
    rid_table_t table;
    vertex_id_t vid;
    vertex_id_t max_vid;

    // Init phase
    this->init();
    iteration_result result = iterator();
    if (0 == result.first.size())
        return std::make_pair(generator_error_t::init_failed_empty_edgeset, table); // initialize failed; returns a empty table
    vid = result.first[0].src;
    max_vid = result.second;
    
    // Iteration
    do
    {
        iteration_per_vertex(table, result.first.size()); 
        vid += 1;

        result = iterator();
        if (0 == result.first.size())
            break; // eof

        if (result.first[0].src > vid)
        {
            for (vertex_id_t id = vid; id < result.first[0].src; ++id)
                iteration_per_vertex(table, 0);
            vid = result.first[0].src;
        }
        
        if (result.second > max_vid) 
            max_vid = result.second;
    } while (true);

    while (max_vid >= vid++)
            iteration_per_vertex(table, 0);
    flush(table);
    return std::make_pair(generator_error_t::success, table);
}

RID_TABLE_GENERATOR_TEMPLATE
typename RID_TABLE_GENERATOR::rid_table_t RID_TABLE_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_edges)
{
    rid_table_t table;
    vertex_id_t src = sorted_edges[0].src;
    vertex_id_t max = (src > sorted_edges[0].dst) ? src : sorted_edges[0].dst;
    ___size_t size = 0;

    auto on_vertex_changed = [this, &table, &src, &size](vertex_id_t new_vid)
    {
        iteration_per_vertex(table, /*src,*/ size);
        size = 0;
        if (new_vid > (src + 1))
            for (vertex_id_t id = src + 1; id <= new_vid; ++id)
                iteration_per_vertex(table, /*id,*/ 0);
    };

    this->init();

    for (___size_t i = 0; i < num_edges; ++i)
    {
        if (sorted_edges[i].dst > max)
            max = sorted_edges[i].dst;
        if (src != sorted_edges[i].src)
        {
            on_vertex_changed(sorted_edges[i].src);
            src = sorted_edges[i].src;
            if (src > max)
                max = src;
        }
        ++size;
    }
    printf("SRC: %u\nMAX: %u\n", src, max); 
    if (max > src)
    {
        src += 1;
        on_vertex_changed(max);
    }
    flush(table);
    return table;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::iteration_per_vertex(rid_table_t& out_table, ___size_t num_edges)
{
    /*while (vid > (vid_counter + 1))
    {
        this->small_page_iteration(out_table, 0);
        ++vid_counter;
    }*/
    if (num_edges > builder_t::MaximumEdgesInHeadPage)
        this->large_page_iteration(out_table, num_edges);
    else
        this->small_page_iteration(out_table, num_edges);
    ++vid_counter;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::flush(rid_table_t& table)
{
    if (!page->is_empty())
        issue_sp(table);
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::small_page_iteration(rid_table_t& table, ___size_t num_edges)
{
    auto  scan_result = page->scan();
    bool& slot_available = scan_result.first;
    auto& capacity = scan_result.second;

    if (!slot_available || (capacity < num_edges))
        issue_sp(table);

    page->add_dummy_slot();
    auto offset = page->number_of_slots() - 1;
    page->add_dummy_list_sp(offset, num_edges);
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::large_page_iteration(rid_table_t& table, ___size_t num_edges)
{
    if (!page->is_empty())
        issue_sp(table);

    ___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - builder_t::MaximumEdgesInHeadPage) / builder_t::MaximumEdgesInExtPage)) + 1;
    issue_lp_head(table, required_ext_pages);
    issue_lp_exts(table, required_ext_pages);
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_sp(rid_table_t& table)
{
    rid_tuple_t tuple;
    tuple.start_vid = next_svid;
    tuple.payload = 0; // small page: 0
    table.push_back(tuple);
    next_svid = vid_counter;
    page->clear();
    ++num_pages;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_lp_head(rid_table_t& table, ___size_t num_related)
{
    rid_tuple_t tuple;
    tuple.start_vid = next_svid;
    tuple.payload = static_cast<typename rid_tuple_t::payload_t>(num_related); // head page: the number of related pages
    table.push_back(tuple);
    // This function does not update a member variable 'last_vid' 
    page->clear();
    ++num_pages;
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::issue_lp_exts(rid_table_t& table, ___size_t num_ext_pages)
{
    rid_tuple_t tuple;
    tuple.start_vid = next_svid;
    for (___size_t i = 1; i <= num_ext_pages; ++i)
    {
        tuple.payload = i; // ext page: page offset from head page
        table.push_back(tuple);
    }
    next_svid = vid_counter + 1;
    page->clear();
    num_pages += num_ext_pages;
}

#undef RID_TABLE_GENERATOR
#undef RID_TABLE_GENERATOR_TEMPLATE

template <typename PageBuilderTy, typename RIDTableTy>
class pagedb_generator
{
public:
    using builder_t = PageBuilderTy;
    ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(builder_t);
    ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(builder_t);
    using rid_table_t = RIDTableTy;
    using rid_tuple_t = typename rid_table_t::value_type;
    using edge_t   = edge_template<vertex_id_t, edge_payload_t>;
    using vertex_t = vertex_template<vertex_id_t, vertex_payload_t>;

    pagedb_generator(rid_table_t& rid_table_);
    
    using edgeset_t = std::vector<edge_t>;
    using edge_iteration_result_t = std::pair<edgeset_t /* sorted vertex #'s edgeset */, vertex_id_t /* max_vid */>;
    using edge_iterator_t = std::function< edge_iteration_result_t() >;
    using vertex_iteration_result_t = std::pair<bool /* success or failure */, vertex_t /* vertex */>;
    using vertex_iterator_t = std::function< vertex_iteration_result_t() >;
    
    // Enabled if vertex_payload_t is void type.
    template <typename PayloadTy = vertex_payload_t>
    typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, std::ofstream& ofs);
    // Enabled if vertex_payload_t is non-void type.
    template <typename PayloadTy = vertex_payload_t>
    typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, vertex_payload_t default_slot_payload, std::ofstream& ofs);

    // Enabled if vertex_payload_t is void type.
    template <typename PayloadTy = vertex_payload_t>
    typename std::enable_if<std::is_void<PayloadTy>::value>::type generate(std::ofstream& ofs, edge_t* sorted_edges, ___size_t num_edges);
    // Enabled if vertex_payload_t is non-void type.
    template <typename PayloadTy = vertex_payload_t>
    typename std::enable_if<!std::is_void<PayloadTy>::value>::type generate(std::ofstream& ofs, vertex_t* sorted_vertices, edge_t* sorted_edges, ___size_t num_edges);

protected:
    void init();
    void iteration_per_vertex(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
    void flush(std::ofstream& ofs);
    void small_page_iteration(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
    void large_page_iteration(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
    void issue_page(std::ofstream& ofs, page_flag_t flags);
    void update_list_buffer(edge_t* edges, ___size_t num_edges);

     rid_table_t& rid_table;
    ___size_t  vid_counter;
    ___size_t  num_pages;
    std::vector<adj_list_elem_t> list_buffer;
    std::shared_ptr<builder_t> page{ std::make_shared<builder_t>() };
};

#define PAGEDB_GENERATOR_TEMPALTE template <typename PageBuilderTy, typename RIDTableTy>
#define PAGEDB_GENERATOR pagedb_generator<PageBuilderTy, RIDTableTy>

PAGEDB_GENERATOR_TEMPALTE
PAGEDB_GENERATOR::pagedb_generator(rid_table_t& rid_table_):
    rid_table{ rid_table_ }
{
    
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::init()
{
    vid_counter = 0;
    num_pages = 0;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generate(edge_iterator_t edge_iterator, std::ofstream& ofs)
{
    vertex_id_t vid;
    vertex_id_t max_vid;

    // Init phase
    this->init();
    edge_iteration_result_t result = edge_iterator();
    if (0 == result.first.size())
        return generator_error_t::init_failed_empty_edgeset; // initialize failed;
    vid = result.first[0].src;
    max_vid = result.second;

    // Iteration
    do
    {
        iteration_per_vertex(ofs, vertex_t{ vid }, result.first.data(), result.first.size());
        vid += 1;

        result = edge_iterator();
        if (0 == result.first.size())
            break; // parsing error?

        if (result.first[0].src > vid)
        {
            for (vertex_id_t id = vid; id < result.first[0].src; ++id)
                iteration_per_vertex(ofs, vertex_t{ id }, result.first.data(), 0);
            vid = result.first[0].src;
        }
            
        if (result.second > max_vid)
            max_vid = result.second;
    }
    while (true);

    while (max_vid >= vid)
        iteration_per_vertex(ofs, vertex_t{ vid++ }, nullptr, 0);

    flush(ofs);
    return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, vertex_payload_t default_slot_payload, std::ofstream& ofs)
{
    vertex_id_t vid;
    vertex_id_t max_vid;

    // Init phase
    this->init();
    edge_iteration_result_t edge_iter_result = edge_iterator();
    vertex_iteration_result_t vertex_iter_result = vertex_iterator();
    if (0 == edge_iter_result.first.size())
        return generator_error_t::init_failed_empty_edgeset; // initialize failed;
    bool& wv_enabled = vertex_iter_result.first;
    vertex_t& wv = vertex_iter_result.second;
    vid = edge_iter_result.first[0].src;
    
    max_vid = edge_iter_result.second;

    // Iteration
    do
    {
        if (!wv_enabled || wv.vertex_id != vid)
        {
            iteration_per_vertex(ofs, vertex_t{ vid, default_slot_payload }, edge_iter_result.first.data(), edge_iter_result.first.size());
            //printf("[DEBUG] Default slot payload applied!\n");
        }
        else
        {
            iteration_per_vertex(ofs, wv, edge_iter_result.first.data(), edge_iter_result.first.size());
            vertex_iter_result = vertex_iterator();
        }
        vid += 1; 

        edge_iter_result = edge_iterator();
        if (0 == edge_iter_result.first.size())
            break; // eof

        if (edge_iter_result.first[0].src > vid)
        {
            for (vertex_id_t id = vid; id < edge_iter_result.first[0].src; ++id)
                iteration_per_vertex(ofs, vertex_t{ id, default_slot_payload }, nullptr, 0);
            vid = edge_iter_result.first[0].src;
        }

        if (edge_iter_result.second > max_vid)
            max_vid = edge_iter_result.second;
    }
    while (true);

    while (max_vid >= vid)
    {
        if (!wv_enabled || wv.vertex_id != vid)
            iteration_per_vertex(ofs, vertex_t{ vid, default_slot_payload }, nullptr, 0);
        else
        {
            iteration_per_vertex(ofs, wv, nullptr, 0);
            vertex_iter_result = vertex_iterator();
        }
        vid += 1;
    }

    flush(ofs);
    return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(std::ofstream& ofs, edge_t* sorted_edges, ___size_t num_edges)
{
    vertex_id_t src = sorted_edges[0].src;
    vertex_id_t max = (src > sorted_edges[0].dst) ? src : sorted_edges[0].dst;
    ___size_t counter = 0;
    ___size_t offset = 0;

    auto on_vertex_changed = [&](vertex_id_t new_vid)
    {
        iteration_per_vertex(ofs, vertex_t{ src }, &sorted_edges[offset], counter);
        offset += counter;
        counter = 0;
        if (new_vid > (src + 1))
            for (vertex_id_t id = src + 1; id <= new_vid; ++id)
            iteration_per_vertex(ofs, vertex_t{ id }, nullptr, 0);
    };

    for (___size_t i = 0; i < num_edges; ++i)
    {
        if (sorted_edges[i].dst > max)
            max = sorted_edges[i].dst;
        if (src != sorted_edges[i].src)
        {
            on_vertex_changed(sorted_edges[i].src);
            src = sorted_edges[i].src;
            if (src > max)
                max = src;
        }
        ++counter;
    }
    printf("SRC: %u\nMAX: %u\n", src, max);
    if (max > src)
    {
        on_vertex_changed(max);
    }
    flush(ofs);
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(std::ofstream& ofs, vertex_t* sorted_vertices, edge_t* sorted_edges, ___size_t num_edges)
{
    vertex_id_t src = sorted_edges[0].src;
    vertex_id_t max = (src > sorted_edges[0].dst) ? src : sorted_edges[0].dst;
    ___size_t counter = 0;
    ___size_t vertex_offset = 0;
    ___size_t edge_offset = 0;

    auto on_vertex_changed = [&](vertex_id_t new_vid)
    {
        iteration_per_vertex(ofs, sorted_vertices[vertex_offset++], &sorted_edges[edge_offset], counter);
        edge_offset += counter;
        counter = 0;
        if (new_vid > (src + 1))
            for (vertex_id_t id = src + 1; id <= new_vid; ++id)
                iteration_per_vertex(ofs, sorted_vertices[id], nullptr, 0);
    };

    for (___size_t i = 0; i < num_edges; ++i)
    {
        if (sorted_edges[i].dst > max)
            max = sorted_edges[i].dst;
        if (src != sorted_edges[i].src)
        {
            on_vertex_changed(sorted_edges[i].src);
            src = sorted_edges[i].src;
            if (src > max)
                max = src;
        }
        ++counter;
    }
    printf("SRC: %u\nMAX: %u\n", src, max);
    if (max > src)
    {
        on_vertex_changed(max);
    }
    flush(ofs);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::iteration_per_vertex(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
    if (num_edges > builder_t::MaximumEdgesInHeadPage)
        this->large_page_iteration(ofs, vertex, edges, num_edges);
    else
        this->small_page_iteration(ofs, vertex, edges, num_edges);
    ++vid_counter;
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::flush(std::ofstream& ofs)
{
    if (!page->is_empty())
        issue_page(ofs, slotted_page_flag::SP);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::small_page_iteration(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
    auto scan_result = page->scan();
    bool& slot_available = scan_result.first;
    auto& capacity = scan_result.second;

    if (!slot_available || (capacity < num_edges))
        issue_page(ofs, slotted_page_flag::SP);

    vertex.to_slot(*page);

    if (num_edges == 0)
        return;

    auto offset = page->number_of_slots() - 1;
    update_list_buffer(edges, num_edges);
    
    page->add_list_sp(offset, list_buffer.data(), num_edges);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::large_page_iteration(std::ofstream& ofs, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
    if (!page->is_empty())
        issue_page(ofs, slotted_page_flag::SP);

    //___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - MaximumEdgesInHeadPage) / MaximumEdgesInExtPage)) + 1;

    // Processing a head page
    {
        constexpr ___size_t num_edges_in_page = MaximumEdgesInHeadPage;
        vertex.to_slot(*page);
        update_list_buffer(edges, num_edges_in_page);
        page->add_list_lp_head(num_edges, list_buffer.data(), num_edges_in_page);
        issue_page(ofs, slotted_page_flag::LP_HEAD);
    }

    // Processing a extended pages
    ___size_t remained_edges = num_edges - MaximumEdgesInHeadPage;
    ___size_t offset = MaximumEdgesInHeadPage;
    {
        ___size_t num_edges_per_page = (remained_edges >= MaximumEdgesInExtPage) ? MaximumEdgesInExtPage : remained_edges;
        vertex.to_slot_ext(*page);
        update_list_buffer(edges + offset, num_edges_per_page);
        page->add_list_lp_ext(list_buffer.data(), num_edges_per_page);
        offset += num_edges_per_page;
        remained_edges -= num_edges_per_page;
        issue_page(ofs, slotted_page_flag::LP_EXTENDED);
    }
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::issue_page(std::ofstream& ofs, page_flag_t flags)
{
    page->flags() = flags;
    builder_t* raw_ptr = page.get();
    ofs.write(reinterpret_cast<char*>(raw_ptr), PageSize);
    page->clear();
    ++num_pages;
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::update_list_buffer(edge_t* edges, ___size_t num_edges)
{
    list_buffer.clear();
    adj_list_elem_t elem;
    for (___size_t i = 0; i < num_edges; ++i)
    {
        edges[i].template to_adj_elem<builder_t>(rid_table, &elem);
        list_buffer.push_back(elem);
    }
}

#undef PAGEDB_GENERATOR
#undef PAGEDB_GENERATOR_TEMPALTE

} // !namespace igraph

#endif // !_INFOGRAPH_PAGEDB_GEN_H_
