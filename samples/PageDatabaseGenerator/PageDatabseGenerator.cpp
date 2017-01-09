#include <infograph/type/slotted_page/pagedb_generator.h>

using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint8_t;
using slot_offset_t = uint8_t;
using adj_list_size_t = uint8_t;
using edge_payload_t = uint8_t;
using vertex_payload_t = uint8_t;
constexpr size_t PageSize = 64;

using vertex_t = igraph::vertex_template<vertex_id_t, edge_payload_t>;
using edge_t = igraph::edge_template<vertex_id_t, edge_payload_t>;

using page_t = igraph::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, adj_list_size_t, PageSize>;
using builder_t = igraph::slotted_page_builder<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, adj_list_size_t, PageSize, edge_payload_t, vertex_payload_t>;
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

int main()
{
    rid_table_generator_t rid_gen;
    printf("Slot Size: %llu\n", builder_t::SlotSize);
    auto table = rid_gen.generate(edge_list.data(), edge_list.size());
    for (auto& tuple : table)
    {
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);
    }

    pagedb_generator_t dbgen{ table };
    std::ofstream ofs{ "test.db", std::ios::out | std::ios::binary };
    dbgen.generate(ofs, vertex_list.data(), edge_list.data(), edge_list.size());
    ofs.close();

    std::ifstream ifs{ "test.db", std::ios::in | std::ios::binary };
    char buffer[PageSize] = { 0, };

    while (!ifs.eof())
    {
        memset(buffer, 0, PageSize);
        ifs.read(buffer, PageSize);
        printf("gcount: %llu\n", ifs.gcount());

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