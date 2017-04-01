#include "utility.h"
#include <gstream/datatype/pagedb.h>
#include <sstream>

// Weighted Edge and Unweighted Vertex: WEUV
namespace weuv {

/* define page arguments */
using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint8_t;
using slot_offset_t = uint8_t;
using record_size_t = uint8_t;
using edge_payload_t = uint8_t; // check
using vertex_payload_t = void;  // check
constexpr std::size_t PageSize = 64;

/* define page type and its helpers (page_traits, generator_traits) */
using page_t = gstream::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
using page_traits = gstream::page_traits<page_t>;
using generator_traits = gstream::generator_traits<page_t>;

std::vector<page_traits::edge_t> weuv_edge_list
{
    // Vertex 0
    { 0x0, 0x1, 0xA },
    { 0x0, 0x6, 0xF },
    // Vertex 1
    { 0x1, 0x0, 0xA },
    { 0x1, 0x1, 0xB },
    { 0x1, 0x6, 0xF },
    // Vertex 2
    { 0x2, 0x3, 0xC },
    { 0x2, 0x6, 0xF },
    // Vertex 3
    { 0x3, 0x2, 0xC },
    { 0x3, 0x6, 0xF },
    // Vertex 4
    { 0x4, 0x5, 0xD },
    { 0x4, 0x6, 0xF },
    // Vertex 5
    { 0x5, 0x4, 0xD },
    { 0x5, 0x6, 0xF },
    // Vertex 6
    { 0x6, 0x0, 0xF },
    { 0x6, 0x1, 0xF },
    { 0x6, 0x2, 0xF },
    { 0x6, 0x3, 0xF },
    { 0x6, 0x4, 0xF },
    { 0x6, 0x5, 0xF },
    { 0x6, 0x7, 0xE },
    { 0x6, 0x8, 0xE },
    { 0x6, 0x9, 0xE },
    { 0x6, 0xA, 0xE },
    { 0x6, 0xB, 0xE },
    { 0x6, 0xC, 0xE },
    { 0x6, 0xD, 0xE },
    { 0x6, 0xE, 0xE },
    { 0x6, 0xF, 0xE },
    { 0x6, 0x10, 0xE },
    { 0x6, 0x11, 0xE },
    { 0x6, 0x12, 0xE }
};

page_traits::edge_t weuv_string_to_edge(std::string& raw)
{
    std::string s = utility::trim(raw);
    std::string tok;
    std::istringstream iss{ s };

    page_traits::edge_t edge;
    std::getline(iss, tok, ' ');
    edge.src = static_cast<vertex_id_t>(std::stoul(tok, 0, 0));
    std::getline(iss, tok, ' ');
    edge.dst = static_cast<vertex_id_t>(std::stoul(tok, 0, 0));
    std::getline(iss, tok, ' ');
    edge.payload = static_cast<edge_payload_t>(std::stoul(tok, 0, 0));
    return edge;
}

/*  Note: The iterator must be a function that does not take parameters.
However, following edge_iterator has one parameter named "ifs" for handling input file.
We can convert it to a function without arguments by using std::bind or lambda. */

// edge and vertex iterator must return a pair, which is consist of vertex #'s edgeset (edge-list) and maximum VID value
// std::pair< std::vector<edge_t> == vertex #'s edgeset, vertex_id_t == maximum VID >
std::pair< std::vector<page_traits::edge_t>, vertex_id_t> weuv_edge_iterator(std::ifstream& ifs)
{
    std::vector<page_traits::edge_t> edgeset;
    std::string buffer;

    // Init phase 
    while (std::getline(ifs, buffer)) // while read a first data elements
    {
        if (buffer[0] == '#' || buffer.length() == 0) // token "#" means comment
            continue; // comment or empty line
        else
            break;
    }
    if (ifs.eof() && 0 == buffer.length())
        return std::make_pair(edgeset, 0); // eof

    page_traits::edge_t edge = weuv_string_to_edge(buffer);
    vertex_id_t src = edge.src;
    vertex_id_t max = (edge.src > edge.dst) ? edge.src : edge.dst;
    edgeset.push_back(edge);

    auto old_pos = ifs.tellg(); // mark an old fpos

    while (std::getline(ifs, buffer)) // read a text file line by line
    {
        if (buffer[0] == '#' || buffer.length() == 0)
        {
            old_pos = ifs.tellg(); // update the old fpos
            continue; // comment or empty line
        }
        edge = weuv_string_to_edge(buffer);
        if (edge.src != src) // check the source vertex id changed
        {
            ifs.seekg(old_pos);
            break; // vertex id is changed, break
        }
        if (edge.dst > max)
            max = edge.dst; // update the maximum vid. As the source VID is fixed, we should compare the variable "max" with current destination value. 
        edgeset.push_back(edge);
        old_pos = ifs.tellg();
    }

    return std::make_pair(edgeset, max);

}

int weuv_disk_based()
{
	/* begin */
	puts("@ Weighted Edge Unweighted Vertex (WEUV) Disk-Based PageDB Geneartion\n");

	/* section: RID-table generator */
	{
		/* open a input file */
		std::ifstream edge_ifs{ "weuv_edges.txt" };

		// create a RID-table generator by gstream::generator_traits
		generator_traits::rid_table_generator_t rtable_generator;
		// call the rid_table_generator::generate method with an edge iterator
		auto generate_result = rtable_generator.generate(std::bind(weuv_edge_iterator, std::ref(edge_ifs)));
		// check the generator returns success
		if (generate_result.error != gstream::generator_error_t::success) {
			puts("Failed to RID Table Generation");
			return -1;
		}
		// save the RID table to file
		std::ofstream ofs{ "weuv_disk_based.rid_table", std::ios::out | std::ios::binary };
		gstream::write_rid_table(generate_result.table, ofs);
		ofs.close();
	}

	/* section: PageDB generator */
	{
		/* open the input files (WEUV: 2 input files) */
		std::ifstream edge_ifs{ "weuv_edges.txt" }; // edge list
		std::ifstream vertex_ifs{ "weuv_vertices.txt" }; // vertex info

														 // read a RID-table from a file for passing to PageDB generator as a constructor argument
		auto rid_table = gstream::read_rid_table<generator_traits::rid_tuple_t, std::vector>("weuv_disk_based.rid_table");
		// create a PageDB generator by gstream::generator_traits with rid_table
		generator_traits::pagedb_generator_t pagedb_generator{ rid_table };
		// call the pagedb_generator::generate method with an in-memory edge list and output stream
		// * Note: the PageDB generator generates a PageDB and writes it to an output stream immediately
		std::ofstream ofs{ "weuv_disk_based.pages", std::ios::out | std::ios::binary };
		pagedb_generator.generate(std::bind(weuv_edge_iterator, std::ref(edge_ifs)) /* edge iterator */, ofs /* output stream */);
		ofs.close();
	}

	/* section: print */
	{
		// read a RID-table from a file
		auto rid_table = gstream::read_rid_table<generator_traits::rid_tuple_t, std::vector>("weuv_disk_based.rid_table");
		// print RID-table
		printf("# RID-table of WEUV\n");
		gstream::print_rid_table(rid_table);

		// read a PageDB from a file
		auto pages = gstream::read_pages<page_t, std::vector>("weuv_disk_based.pages");
		// print PageDB
		printf("\n# PageDB of WEUV\n");
		for (std::size_t i = 0; i < pages.size(); ++i) {
			printf("page[%llu]--------------------------------\n", i);
			gstream::print_page(pages[i]);
			printf("\n");
		}
	}
	return 0;
}

int weuv_in_memory()
{
	/* begin */
	puts("@ Weighted Edge Weighted Vertex (WEUV) In-Memory PageDB Geneartion\n");

	/* section: RID-table generator */
	{
		// create a RID-table generator by gstream::generator_traits
		generator_traits::rid_table_generator_t rtable_generator;
		// call the rid_table_generator::generate method with an in-memory edge list
		auto generate_result = rtable_generator.generate(weuv_edge_list.data(), weuv_edge_list.size());
		// check the generator returns success
		if (generate_result.error != gstream::generator_error_t::success) {
			puts("Failed to RID Table Generation");
			return -1;
		}
		// save the RID table to file
		std::ofstream ofs{ "weuv_inmemory.rid_table", std::ios::out | std::ios::binary };
		gstream::write_rid_table(generate_result.table, ofs);
		ofs.close();
	}

	/* section: PageDB generator */
	{
		// read a RID-table from a file for passing to PageDB generator as a constructor argument
		auto rid_table = gstream::read_rid_table<generator_traits::rid_tuple_t, std::vector>("weuv_inmemory.rid_table");
		// create a PageDB generator by gstream::generator_traits with rid_table
		generator_traits::pagedb_generator_t pagedb_generator{ rid_table };
		// call the pagedb_generator::generate method with an in-memory edge list and output stream
		// * Note: the PageDB generator generates a PageDB and writes it to an output stream immediately
		std::ofstream ofs{ "weuv_inmemory.pages", std::ios::out | std::ios::binary };
		pagedb_generator.generate(weuv_edge_list.data(), weuv_edge_list.size(), ofs);
		ofs.close();
	}

	/* section: print */
	{
		// read a RID-table from a file
		auto rid_table = gstream::read_rid_table<generator_traits::rid_tuple_t, std::vector>("weuv_inmemory.rid_table");
		// print RID-table
		printf("# RID-table of WEUV\n");
		gstream::print_rid_table(rid_table);

		// read a PageDB from a file
		auto pages = gstream::read_pages<page_t, std::vector>("weuv_inmemory.pages");
		// print PageDB
		printf("\n# PageDB of WEUV\n");
		for (std::size_t i = 0; i < pages.size(); ++i) {
			printf("page[%llu]--------------------------------\n", i);
			gstream::print_page(pages[i]);
			printf("\n");
		}
	}
	return 0;
}

} // !namespace weuv