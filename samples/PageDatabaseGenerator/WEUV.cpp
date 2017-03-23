#include "utility.h"
#include <gstream/datatype/pagedb_generator.h>
#include <sstream>

namespace weuv {

using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint16_t;
using slot_offset_t = uint8_t;
using record_size_t = uint8_t;
using edge_payload_t = uint8_t; // check
using vertex_payload_t = void;  // check
constexpr size_t PageSize = 64;

using vertex_t = gstream::vertex_template<vertex_id_t, vertex_payload_t>;
using edge_t = gstream::edge_template<vertex_id_t, edge_payload_t>;

using page_t = gstream::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
using builder_t = gstream::slotted_page_builder<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;

using rid_table_generator_t = gstream::rid_table_generator<builder_t>;
using pagedb_generator_t = gstream::pagedb_generator<builder_t, rid_table_generator_t::rid_table_t>;

std::vector<edge_t> edge_list
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

edge_t string_to_edge(std::string& raw)
{
    std::string s = utility::trim(raw);
    std::string tok;
    std::istringstream iss{ s };

    edge_t edge;
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
std::pair< std::vector<edge_t>, vertex_id_t> edge_iterator(std::ifstream& ifs)
{
    std::vector<edge_t> edgeset;
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

    edge_t edge = string_to_edge(buffer);
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
        edge = string_to_edge(buffer);
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

int test_disk_based()
{
    puts("[Weighted Edge Weighted Vertex (WEUV) Disk-Based PageDB Geneartion]");
    std::ifstream edge_ifs{ "weuv_edges.txt" };

    rid_table_generator_t rid_gen;
    auto result = rid_gen.generate(std::bind(edge_iterator, std::ref(edge_ifs)));
    if (result.first != gstream::generator_error_t::success)
    {
        puts("Failed to RID Table Generation");
        return -1;
    }

    std::ofstream rid_out{ "weuv_disk_based.rid_table", std::ios::out | std::ios::binary };
    for (auto& tuple : result.second)
    {
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
        rid_out.write(reinterpret_cast<char*>(&tuple.start_vid), sizeof(tuple.start_vid));
        rid_out.write(reinterpret_cast<char*>(&tuple.payload), sizeof(tuple.payload));
    }
    printf("\n");
    rid_out.close();

    edge_ifs.clear();
    edge_ifs.seekg(0);

    pagedb_generator_t dbgen{ result.second };
    std::ofstream ofs{ "weuv_disk_based.pages", std::ios::out | std::ios::binary };
    dbgen.generate(std::bind(edge_iterator, std::ref(edge_ifs)), // edge iterator
                   ofs);    // output stream
    ofs.close();

    std::ifstream ifs{ "weuv_disk_based.pages", std::ios::in | std::ios::binary };
    
    char buffer[PageSize] = { 0, };
    uint64_t pid = 0;
    while (!ifs.eof())
    {
        memset(buffer, 0, PageSize);
        ifs.read(buffer, PageSize);
        printf("gcount: %llu, PID[%llu]\n", ifs.gcount(), pid++);

        for (int i = 1; i <= ifs.gcount(); ++i)
        {
            printf("0x%02X ", buffer[i - 1]);
            if (i % 8 == 0)
                printf("\n");
        }
        printf("\n");
    }
    ifs.close();

    return 0;
}

int test_in_memory()
{
    puts("[Weighted Edge Weighted Vertex (WEUV) In-Memory PageDB Geneartion]");
    rid_table_generator_t rid_gen;
    auto result = rid_gen.generate(edge_list.data(), edge_list.size());
    if (result.first != gstream::generator_error_t::success)
    {
        puts("Failed to RID Table Generation");
        return -1;
    }

    rid_table_generator_t::rid_table_t& rid_table = result.second;
    std::ofstream rid_out{ "weuv_inmemory.rid_table", std::ios::out | std::ios::binary };
    for (auto& tuple : rid_table)
    {
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
        rid_out.write(reinterpret_cast<char*>(&tuple.start_vid), sizeof(tuple.start_vid));
        rid_out.write(reinterpret_cast<char*>(&tuple.payload), sizeof(tuple.payload));
    }
    printf("\n");
    rid_out.close();

    pagedb_generator_t dbgen{ rid_table };
    std::ofstream ofs{ "weuv_inmemory.pages", std::ios::out | std::ios::binary };
    dbgen.generate(edge_list.data(), edge_list.size(), ofs);
    ofs.close();

    std::ifstream ifs{ "weuv_inmemory.pages", std::ios::in | std::ios::binary };
    char buffer[PageSize] = { 0, };

    uint64_t pid = 0;
    while (!ifs.eof())
    {
        memset(buffer, 0, PageSize);
        ifs.read(buffer, PageSize);
        printf("gcount: %llu, PID[%llu]\n", ifs.gcount(), pid++);

        for (int i = 1; i <= ifs.gcount(); ++i)
        {
            printf("0x%02X ", buffer[i - 1]);
            if (i % 8 == 0)
                printf("\n");
        }
        printf("\n");
    }
    ifs.close();

    return 0;
}

} // !namespace weuv