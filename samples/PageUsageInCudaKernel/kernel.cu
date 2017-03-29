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
using page_traits = gstream::page_traits<page_t>;
using generator_traits = gstream::generator_traits<page_t>;

// Define container type for storing pages (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
using page_cont_t = std::vector<page_t>;
// Define RID tuple type
using rid_tuple_t = generator_traits::rid_tuple_t;
// Define RID table type (you can use any type of STL sequential container for constructing RID table, e.g., std::vector, std::list)
using rid_table_t = generator_traits::rid_table_t;

int main()
{
	rid_table_t rtable = gstream::read_rid_table<rid_tuple_t, std::vector>("wewv.rid_table");
	printf("# RID Table\n");
	gstream::print_rid_table(rtable);

	page_cont_t pages = gstream::read_pages<page_t, std::vector>("wewv.pages");
	printf("\n# Pages\n");
	for (size_t i = 0; i < pages.size(); ++i) {
		printf("page[%llu]--------------------------------\n", i);
		gstream::print_page(pages[i]);
		printf("\n");
	}

	return 0;
}