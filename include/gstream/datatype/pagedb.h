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
template <typename __vertex_id_t, typename __payload_t = std::size_t>
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
CONT_T<PAGE_T> read_pages(const char* filepath, const std::size_t bundle_of_pages = 64)
{
    using page_t = PAGE_T;
    using cont_t = CONT_T<PAGE_T>;

    // Open a file stream
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };

    // TODO: metadata implementation

    
    cont_t pages; // container for pages which will be returned.

    // Read pages
    {
        const std::size_t chunk_size = sizeof(page_t) * bundle_of_pages;
        std::vector<page_t> buffer;
        buffer.resize(bundle_of_pages);
        uint64_t counter = 0;
        while (true)
        {
            ++counter;
            ifs.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
            std::size_t extracted = ifs.gcount() / sizeof(page_t);
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
       /* printf("Finished read %llu pages from file %s. bundle size = %llu, total iteration: %llu\n",
               pages.size(),
               filepath,
               bundle_of_pages,
               counter);*/
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
        /*printf("Finished read RID Table from file %s. number of tuples: %llu\n",
               filepath,
               table.size());*/
    }

    return table;
}

template <typename PageTy>
struct page_traits {
	using page_t = PageTy;
	using vertex_id_t = typename page_t::vertex_id_t;
	using page_id_t = typename page_t::page_id_t;
	using record_offset_t = typename page_t::record_offset_t;
	using slot_offset_t = typename page_t::slot_offset_t;
	using record_size_t = typename page_t::record_size_t;
	using edge_payload_t = typename page_t::edge_payload_t;
	using vertex_payload_t = typename page_t::vertex_payload_t;
	static constexpr std::size_t PageSize = page_t::PageSize;
	using vertex_t = vertex_template<vertex_id_t, vertex_payload_t>;
	using edge_t = edge_template<vertex_id_t, edge_payload_t>;
	using page_builder_t = slotted_page_builder<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
};

enum class generator_error_t {
	success,
	init_failed_empty_edgeset,
};

template <typename PageTy,
	typename RIDTuplePayloadTy = std::size_t,
	template <typename _ElemTy,
	typename = std::allocator<_ElemTy> >
	class RIDTupleContTy = std::vector >
	class rid_table_generator {
	public:
		using page_t = PageTy;
		using page_traits = page_traits<page_t>;
		using page_builder_t = typename page_traits::page_builder_t;
		ALIAS_SLOTTED_PAGE_TEMPLATE_TYPEDEFS(page_builder_t);
		ALIAS_SLOTTED_PAGE_TEMPLATE_CONSTDEFS(page_builder_t);
		using rid_tuple_t = rid_tuple_template<typename page_traits::vertex_id_t, RIDTuplePayloadTy>;
		using cont_t = RIDTupleContTy<rid_tuple_t>;
		using rid_table_t = cont_t;
		using edge_t = edge_template<vertex_id_t, edge_payload_t>;

		using edgeset_t = std::vector<edge_t>;
		using edge_iteration_result_t = std::pair<edgeset_t /* sorted vertex #'s edgeset */, vertex_id_t /* max_vid */>;
		using edge_iterator_t = std::function< edge_iteration_result_t() >;
		struct generate_result {
			generator_error_t error;
			rid_table_t table;
		};
		generate_result generate(edge_iterator_t edge_iterator);
		generate_result generate(edge_t* sorted_edges, ___size_t num_edges);

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
		std::shared_ptr<page_builder_t> page{ std::make_shared<page_builder_t>() };
};

#define RID_TABLE_GENERATOR_TEMPLATE template <typename PageTy, typename RIDTuplePayloadTy, template <typename _ElemTy,	typename = std::allocator<_ElemTy> > class RIDTupleContTy = std::vector >
#define RID_TABLE_GENERATOR rid_table_generator<PageTy, RIDTuplePayloadTy, RIDTupleContTy>

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::init()
{
	next_svid = 0;
	vid_counter = 0;
	num_pages = 0;
}

RID_TABLE_GENERATOR_TEMPLATE
typename RID_TABLE_GENERATOR::generate_result RID_TABLE_GENERATOR::generate(edge_iterator_t iterator)
{
	rid_table_t table;
	vertex_id_t vid;
	vertex_id_t max_vid;

	// Init phase
	this->init();
	edge_iteration_result_t eir = iterator();
	if (0 == eir.first.size())
		return generate_result{ generator_error_t::init_failed_empty_edgeset, table }; // initialize failed; returns a empty table
	vid = eir.first[0].src;
	max_vid = eir.second;

	// Iteration
	do
	{
		iteration_per_vertex(table, eir.first.size());
		vid += 1;

		eir = iterator();
		if (0 == eir.first.size())
			break; // eof

		if (eir.first[0].src > vid)
		{
			for (vertex_id_t id = vid; id < eir.first[0].src; ++id)
				iteration_per_vertex(table, 0);
			vid = eir.first[0].src;
		}

		if (eir.second > max_vid)
			max_vid = eir.second;
	} while (true);

	while (max_vid >= vid++)
		iteration_per_vertex(table, 0);
	flush(table);
	return generate_result{ generator_error_t::success, table };
}

RID_TABLE_GENERATOR_TEMPLATE
typename RID_TABLE_GENERATOR::generate_result RID_TABLE_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_total_edges)
{
	___size_t off = 0;
	vertex_id_t src;
	vertex_id_t max;
	auto edge_iterator = [&]() -> edge_iteration_result_t
	{
		edgeset_t edgeset;
		if (off == num_total_edges)
			return std::make_pair(edgeset, 0); // eof

		src = sorted_edges[off].src;
		max = (sorted_edges[off].src > sorted_edges[off].dst) ? sorted_edges[off].src : sorted_edges[off].dst;
		edgeset.push_back(sorted_edges[off++]);
		for (___size_t i = off; i < num_total_edges; ++i)
		{
			if (sorted_edges[off].src == src)
			{
				if (sorted_edges[off].dst > max)
					max = sorted_edges[off].dst;
				edgeset.push_back(sorted_edges[off++]);
			}
			else break;
		}
		return std::make_pair(edgeset, max);
	};
	return this->generate(edge_iterator);
}

RID_TABLE_GENERATOR_TEMPLATE
void RID_TABLE_GENERATOR::iteration_per_vertex(rid_table_t& out_table, ___size_t num_edges)
{
	/*while (vid > (vid_counter + 1))
	{
	this->small_page_iteration(out_table, 0);
	++vid_counter;
	}*/
	if (num_edges > page_builder_t::MaximumEdgesInHeadPage)
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

	___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - page_builder_t::MaximumEdgesInHeadPage) / page_builder_t::MaximumEdgesInExtPage)) + 1;
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
	using edge_t = edge_template<vertex_id_t, edge_payload_t>;
	using vertex_t = vertex_template<vertex_id_t, vertex_payload_t>;

	pagedb_generator(rid_table_t& rid_table_);

	using edgeset_t = std::vector<edge_t>;
	using edge_iteration_result_t = std::pair<edgeset_t /* sorted vertex #'s edgeset */, vertex_id_t /* max_vid */>;
	using edge_iterator_t = std::function< edge_iteration_result_t() >;
	using vertex_iteration_result_t = std::pair<bool /* success or failure */, vertex_t /* vertex */>;
	using vertex_iterator_t = std::function< vertex_iteration_result_t() >;

	// Enabled if vertex_payload_t is void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, std::ostream& os);
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os);

	// Enabled if vertex_payload_t is void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<std::is_void<PayloadTy>::value>::type generate(edge_t* sorted_edges, ___size_t num_edges, std::ostream& os);
	// Enabled if vertex_payload_t is non-void type.
	template <typename PayloadTy = vertex_payload_t>
	typename std::enable_if<!std::is_void<PayloadTy>::value>::type generate(edge_t* sorted_edges, ___size_t num_edges, vertex_t* sorted_vertices, ___size_t num_vertices, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os);

protected:
	void init();
	void iteration_per_vertex(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
	void flush(std::ostream& os);
	void small_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
	void large_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges);
	void issue_page(std::ostream& os, page_flag_t flags);
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
PAGEDB_GENERATOR::pagedb_generator(rid_table_t& rid_table_) :
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
typename std::enable_if<std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generate(edge_iterator_t edge_iterator, std::ostream& os)
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
		iteration_per_vertex(os, vertex_t{ vid }, result.first.data(), result.first.size());
		vid += 1;

		result = edge_iterator();
		if (0 == result.first.size())
			break; // parsing error?

		if (result.first[0].src > vid)
		{
			for (vertex_id_t id = vid; id < result.first[0].src; ++id)
				iteration_per_vertex(os, vertex_t{ id }, result.first.data(), 0);
			vid = result.first[0].src;
		}

		if (result.second > max_vid)
			max_vid = result.second;
	} while (true);

	while (max_vid >= vid)
		iteration_per_vertex(os, vertex_t{ vid++ }, nullptr, 0);

	flush(os);
	return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value, generator_error_t>::type PAGEDB_GENERATOR::generate(edge_iterator_t edge_iterator, vertex_iterator_t vertex_iterator, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os)
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
			iteration_per_vertex(os, vertex_t{ vid, default_slot_payload }, edge_iter_result.first.data(), edge_iter_result.first.size());
		}
		else
		{
			iteration_per_vertex(os, wv, edge_iter_result.first.data(), edge_iter_result.first.size());
			vertex_iter_result = vertex_iterator();
		}
		vid += 1;

		edge_iter_result = edge_iterator();
		if (0 == edge_iter_result.first.size())
			break; // eof

		if (edge_iter_result.first[0].src > vid)
		{
			for (vertex_id_t id = vid; id < edge_iter_result.first[0].src; ++id)
				iteration_per_vertex(os, vertex_t{ id, default_slot_payload }, nullptr, 0);
			vid = edge_iter_result.first[0].src;
		}

		if (edge_iter_result.second > max_vid)
			max_vid = edge_iter_result.second;
	} while (true);

	while (max_vid >= vid)
	{
		if (!wv_enabled || wv.vertex_id != vid)
			iteration_per_vertex(os, vertex_t{ vid, default_slot_payload }, nullptr, 0);
		else
		{
			iteration_per_vertex(os, wv, nullptr, 0);
			vertex_iter_result = vertex_iterator();
		}
		vid += 1;
	}

	flush(os);
	return generator_error_t::success;
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_total_edges, std::ostream& os)
{
	___size_t off = 0;
	vertex_id_t src;
	vertex_id_t max;
	auto edge_iterator = [&]() -> edge_iteration_result_t
	{
		edgeset_t edgeset;
		if (off == num_total_edges)
			return std::make_pair(edgeset, 0); // eof

		src = sorted_edges[off].src;
		max = (sorted_edges[off].src > sorted_edges[off].dst) ? sorted_edges[off].src : sorted_edges[off].dst;
		edgeset.push_back(sorted_edges[off++]);
		for (___size_t i = off; i < num_total_edges; ++i)
		{
			if (sorted_edges[off].src == src)
			{
				if (sorted_edges[off].dst > max)
					max = sorted_edges[off].dst;
				edgeset.push_back(sorted_edges[off++]);
			}
			else break;
		}
		return std::make_pair(edgeset, max);
	};

	this->generate(edge_iterator, os);
}

PAGEDB_GENERATOR_TEMPALTE
template <typename PayloadTy>
typename std::enable_if<!std::is_void<PayloadTy>::value>::type PAGEDB_GENERATOR::generate(edge_t* sorted_edges, ___size_t num_total_edges, vertex_t* sorted_vertices, ___size_t num_vertices, typename std::enable_if< !std::is_void<PayloadTy>::value, PayloadTy >::type default_slot_payload, std::ostream& os)
{
	___size_t e_off = 0;
	vertex_id_t src;
	vertex_id_t max;
	auto edge_iterator = [&]() -> edge_iteration_result_t
	{
		edgeset_t edgeset;
		if (e_off == num_total_edges)
			return std::make_pair(edgeset, 0); // eof

		src = sorted_edges[e_off].src;
		max = (sorted_edges[e_off].src > sorted_edges[e_off].dst) ? sorted_edges[e_off].src : sorted_edges[e_off].dst;
		edgeset.push_back(sorted_edges[e_off++]);
		for (___size_t i = e_off; i < num_total_edges; ++i)
		{
			if (sorted_edges[e_off].src == src)
			{
				if (sorted_edges[e_off].dst > max)
					max = sorted_edges[e_off].dst;
				edgeset.push_back(sorted_edges[e_off++]);
			}
			else break;
		}
		return std::make_pair(edgeset, max);
	};

	___size_t v_off = 0;
	auto vertex_iterator = [&]() -> vertex_iteration_result_t
	{
		if (v_off == num_vertices)
			return std::make_pair(false, vertex_t{});
		return std::make_pair(true, sorted_vertices[v_off++]);
	};

	this->generate(edge_iterator, vertex_iterator, default_slot_payload, os);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::iteration_per_vertex(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	if (num_edges > builder_t::MaximumEdgesInHeadPage)
		this->large_page_iteration(os, vertex, edges, num_edges);
	else
		this->small_page_iteration(os, vertex, edges, num_edges);
	++vid_counter;
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::flush(std::ostream& os)
{
	if (!page->is_empty())
		issue_page(os, slotted_page_flag::SP);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::small_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	auto scan_result = page->scan();
	bool& slot_available = scan_result.first;
	auto& capacity = scan_result.second;

	if (!slot_available || (capacity < num_edges))
		issue_page(os, slotted_page_flag::SP);

	vertex.to_slot(*page);

	if (num_edges == 0)
		return;

	auto offset = page->number_of_slots() - 1;
	update_list_buffer(edges, num_edges);

	page->add_list_sp(offset, list_buffer.data(), num_edges);
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::large_page_iteration(std::ostream& os, const vertex_t& vertex, edge_t* edges, ___size_t num_edges)
{
	if (!page->is_empty())
		issue_page(os, slotted_page_flag::SP);

	//___size_t required_ext_pages = static_cast<___size_t>(std::floor((num_edges - MaximumEdgesInHeadPage) / MaximumEdgesInExtPage)) + 1;

	// Processing a head page
	{
		constexpr ___size_t num_edges_in_page = MaximumEdgesInHeadPage;
		vertex.to_slot(*page);
		update_list_buffer(edges, num_edges_in_page);
		page->add_list_lp_head(num_edges, list_buffer.data(), num_edges_in_page);
		issue_page(os, slotted_page_flag::LP_HEAD);
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
		issue_page(os, slotted_page_flag::LP_EXTENDED);
	}
}

PAGEDB_GENERATOR_TEMPALTE
void PAGEDB_GENERATOR::issue_page(std::ostream& os, page_flag_t flags)
{
	page->flags() = flags;
	builder_t* raw_ptr = page.get();
	os.write(reinterpret_cast<char*>(raw_ptr), PageSize);
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

template <typename PageTy, typename RIDTuplePayloadTy = std::size_t, template <typename _ElemTy, typename = std::allocator<_ElemTy> > class RIDContainerTy = std::vector>
struct generator_traits {
	using page_t = PageTy;
	using page_traits = page_traits<PageTy>;
	using rid_table_generator = rid_table_generator<PageTy, RIDTuplePayloadTy, RIDContainerTy>;
	using rid_tuple_t = typename rid_table_generator::rid_tuple_t;
	using pagedb_generator = pagedb_generator<typename page_traits::page_builder_t, typename rid_table_generator::rid_table_t>;
};

template <typename RIDTableTy>
void write_rid_table(RIDTableTy& rid_table, std::ostream& os)
{
	for (auto& tuple : rid_table) {
		os.write(reinterpret_cast<char*>(&tuple.start_vid), sizeof(tuple.start_vid));
		os.write(reinterpret_cast<char*>(&tuple.payload), sizeof(tuple.payload));
	}
}

template <typename PageTy>
void print_page(PageTy& page)
{
	printf("number of slots in the page: %u\n", page.number_of_slots());
	printf("page type: %s\n",
		(page.is_sp()) ?
		"small page" : (page.is_lp_head()) ?
		"large page (head)" : "large page (extended)");
	for (int i = 1; i <= PageTy::PageSize; ++i) {
		printf("0x%02X ", page[i - 1]);
		if (i % 8 == 0)
			printf("\n");
	}
}

template <typename RIDTableTy>
void print_rid_table(RIDTableTy& rid_table)
{
	for (auto& tuple : rid_table)
		printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
}

#undef PAGEDB_GENERATOR
#undef PAGEDB_GENERATOR_TEMPALTE

} // !nameaspace gstream

#endif // !_GSTREAM_DATATYPE_PAGEDB_H_
