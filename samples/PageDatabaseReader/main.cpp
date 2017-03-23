#include <gstream/datatype/pagedb.h>

// Define meta parameter for page type 
using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint8_t;
using slot_offset_t = uint8_t;
using record_size_t = uint8_t;
using edge_payload_t = uint8_t;
using vertex_payload_t = uint8_t;
constexpr size_t PageSize = 64;

// Define page type
using page_t = gstream::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
// Define container type for storing pages (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
using page_cont_t = std::vector<page_t>;
// Define RID tuple type
using rid_tuple_t = gstream::rid_tuple_template<vertex_id_t>;
// Define RID table type (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
using rid_table_t = std::vector<rid_tuple_t>;

int main()
{
    rid_table_t rid_table = gstream::read_rid_table<rid_tuple_t, std::vector>("wewv.rid_table");
    page_cont_t pages = gstream::read_pages<page_t, std::vector>("wewv.pages");
    
    printf("# RID Table\n");
    for (auto& tuple : rid_table)
        printf("%u\t|\t%llu\n", tuple.start_vid, tuple.payload);

    printf("\n# Pages\n");
    for (size_t i = 0; i < pages.size(); ++i)
    {
        unsigned char* buffer = reinterpret_cast<unsigned char*>(&pages[i]);
        printf("page[%llu]--------------------------------\n", i);
        for (int j = 1; j <= sizeof(page_t); ++j)
        {
            printf("0x%02X ", buffer[j - 1]);
            if (j % 8 == 0)
                printf("\n");
        }
    }

    return 0;
}