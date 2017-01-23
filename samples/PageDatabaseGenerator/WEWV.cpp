#include "utility.h"
#include <infograph/type/slotted_page/pagedb_generator.h>
#include <sstream>

namespace wewv {

using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint8_t;
using slot_offset_t = uint8_t;
using record_size_t = uint8_t;
using edge_payload_t = uint8_t;
using vertex_payload_t = uint8_t;
constexpr size_t PageSize = 64;

using vertex_t = igraph::vertex_template<vertex_id_t, edge_payload_t>;
using edge_t = igraph::edge_template<vertex_id_t, edge_payload_t>;

using page_t = igraph::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize>;
using builder_t = igraph::slotted_page_builder<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
using rid_table_generator_t = igraph::rid_table_generator<builder_t>;
using pagedb_generator_t = igraph::pagedb_generator<builder_t, rid_table_generator_t::rid_table_t>;

std::vector<vertex_t> vertex_list
{
    { 0x0, 0x0 },
    { 0x1, 0x1 },
    { 0x2, 0x2 },
    { 0x3, 0x3 },
    { 0x4, 0x4 },
    { 0x5, 0x5 },
    { 0x6, 0x6 },
    { 0x7, 0x7 },
    { 0x8, 0x8 },
    { 0x9, 0x9 },
    { 0xA, 0xA },
    { 0xB, 0xB },
    { 0xC, 0xC },
    { 0xD, 0xD },
    { 0xE, 0xE },
    { 0xF, 0xF },
    { 0x10, 0x10 },
    { 0x11, 0x11 },
    { 0x12, 0x12 }
};

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

vertex_t string_to_vertex(std::string& raw)
{
    std::string s = utility::trim(raw);
    std::string tok;
    std::istringstream iss{ s };

    vertex_t vertex;
    std::getline(iss, tok, ' ');
    vertex.vertex_id = static_cast<vertex_id_t>(std::stoul(tok, 0, 0));
    std::getline(iss, tok, ' ');
    vertex.payload = static_cast<vertex_payload_t>(std::stoul(tok, 0, 0));

    return vertex;
}

rid_table_generator_t::iteration_result edge_iterator(std::ifstream& ifs)
{
    rid_table_generator_t::edgeset_t edgeset;
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
    printf("[E]\t0x%02X -> 0x%02X\tW: 0x%02X\n", edge.src, edge.dst, edge.payload);
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

        printf("[E]\t0x%02X -> 0x%02X\tW: 0x%02X\n", edge.src, edge.dst, edge.payload);
    }

    return std::make_pair(edgeset, max);
}

std::pair<bool, vertex_t> vertex_iterator(std::ifstream& ifs)
{
    std::string buffer;
    while (std::getline(ifs, buffer)) // read a text file line by line
    {
        if (buffer[0] == '#' || buffer.length() == 0)
            continue; // comment or empty line
        vertex_t v = string_to_vertex(buffer);
        printf("[V]\t0x%02X\tW: 0x%02X\n", v.vertex_id, v.payload);
        return std::make_pair(true, v);
    }
    return std::make_pair(false, vertex_t{ 0, 0 });
}

int test_disk_based()
{
    std::ifstream edge_ifs{ "wewv_edges.txt" };
    std::ifstream vertex_ifs{ "wewv_vertices.txt" };

    rid_table_generator_t rid_gen;
    printf("Slot Size: %llu\n", builder_t::SlotSize);

    auto result = rid_gen.generate(std::bind(edge_iterator, std::ref(edge_ifs)));
    if (result.first != igraph::generator_error_t::success)
    {
        puts("Failed to RID Table Generation");
        return -1;
    }
    for (auto& tuple : result.second)
    {
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
    }

    edge_ifs.clear();
    edge_ifs.seekg(0);

    pagedb_generator_t dbgen{ result.second };
    std::ofstream ofs{ "wewv_disk_based.db", std::ios::out | std::ios::binary };
    dbgen.generate(std::bind(edge_iterator, std::ref(edge_ifs)),        // edge iterator
                   std::bind(vertex_iterator, std::ref(vertex_ifs)),    // vertex iterator
                   0xCC,    // default vertex payload
                   ofs);    // output stream
    ofs.close();

    std::ifstream ifs{ "wewv_disk_based.db", std::ios::in | std::ios::binary };
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
    rid_table_generator_t rid_gen;
    printf("Slot Size: %llu\n", builder_t::SlotSize);
    auto table = rid_gen.generate(edge_list.data(), edge_list.size());
    for (auto& tuple : table)
    {
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
    }

    pagedb_generator_t dbgen{ table };
    std::ofstream ofs{ "wewv_inmemory.db", std::ios::out | std::ios::binary };
    dbgen.generate(ofs, vertex_list.data(), edge_list.data(), edge_list.size());
    ofs.close();

    std::ifstream ifs{ "wewv_inmemory.db", std::ios::in | std::ios::binary };
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

} // !namespace wewv