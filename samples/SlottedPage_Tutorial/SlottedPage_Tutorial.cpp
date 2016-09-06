#include <infograph/type/generic/slotted_page.h>
#include <infograph/type/generic/slotted_page_helper.h>

/* ---------------------------------------------------------------
**
** Slotted page represntation (Page size: user defined)
** +-------------------------------------------------------------+
** |                                                             |
** |                                                             |
** |                      Data Section                           |
** |               size = page size - footer size       +--------+
** |                                                    | footer |
** +----------------------------------------------------+--------+
**
**  Detailed slotted page representation
** +-------------------------------------------------------------+
** | S0 adj-list size | S0 adj-elem #0 | S0 adj-elem #1 | S0 adj-|
** +-------------------------------------------------------------+
** |elem #2 | S0 adj-elem #3 | ...                               |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                ... | S0 adj-elem #N | S1 adj-list size | S1 |
** +-------------------------------------------------------------+
** | adj-elem #0 | S2 adj-elem #1 | S1 adj-elem #2 | S1 adj-elem |
** +-------------------------------------------------------------+
** | #3 | ...                                                    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                                    ...| S1 adj-elem #M |    |
** +-------------------------------------------------------------+
** |                                                             |
** +-------------------------------------------------------------+
** |                 | slot #1 (S1) | slot #0 (S0) | page footer |
** +-------------------------------------------------------------+
**
** ------------------------------------------------------------ */

/* Slot representation in the slotted page
** +--------------------------------------------------------+
** | vertex_id (user-def type) | internal_offset (uint32_t) |
** +--------------------------------------------------------+ */
using vertex_id_t = uint32_t;

/* Adjacency element representation
** +---------------------------------------------------------------------------------------+
** | adj_page_id(user - def type) | adj_offset(user - def type) | payload(user - def type) |
** +---------------------------------------------------------------------------------------+ */
using adj_page_id_t = uint16_t;
using adj_offset_t  = uint16_t;
using adj_payload_t = uint16_t;

/* Adjacency list size: user defined type
** +---------------------------------------------+
** | Slot #@ adjacency list size (user-def type) |
** +---------------------------------------------+ */
using adj_list_size_t = uint32_t;

/* Slotted page typedef */
const size_t PAGE_SIZE = 128;
using test_page_t =
igraph::slotted_page<
    vertex_id_t,
    adj_page_id_t,
    adj_offset_t,
    adj_list_size_t,
    PAGE_SIZE,
    adj_payload_t
>;

namespace {

inline void print_line()
{
    puts("-------------------------------------------------------------------------------");
}

}

void test_single_page();
void test_large_page();

int main()
{
    test_single_page();
    test_large_page();
    return 0;
}

void test_single_page()
{
    print_line();
    puts("Single page test");
    print_line();

    // Generate slotted_page instance (using shared_ptr recommended as follows for single page handling)
    auto page = std::make_shared<test_page_t>(igraph::page_flag::SMALL_PAGE);

    // Set a mode of the page
    page->footer.flags |= igraph::page_flag::SMALL_PAGE;

    print_line();
    printf("Byte level representation of an empty slotted page (page size: %llu)\n", PAGE_SIZE);
    print_line();
    igraph::print_page<16>(*page);
    print_line();

    // Test slotted_page::is_empty()
    {
        bool result = page->is_empty();
        printf("slotted_page::is_empty() = %s\n", result ? "true" : "false");
        print_line();
    }

    // Test slotted_page::number_of_slots()
    {
        size_t result = page->number_of_slots();
        printf("@ Get a number of slots stored in a slotted page\n");
        printf("slotted_page::number_of_slots() = %llu\n", result);
        print_line();
    }

    // Test slotted_page::scan()
    {
        auto result = page->scan();
        printf("@ Scan operation: scans free space in the slotted page, and returns result of a couple of informations as follows:\n\
(1) Whether it is possible to store new slot to this page or not (boolean)\n\
(2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)\n");
        printf("slotted_page::scan() = %s | %llu\n", result.first ? "true" : "false", result.second);
        print_line();
    }

    // Test slotted_page::add_slot()
    {
        auto result = page->add_slot(0x0A /* vertex id */);
        printf("@ Add slot operation (slot #0): add a slot into slotted page, this operation returns,\n\
(1) Operation result (boolean)\n\
(2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)\n");
        printf("# Slot representation in the slotted page\n\
\t+--------------------------------------------------------+\n\
\t| vertex_id (user-def type) | internal_offset (uint32_t) |\n\
\t+--------------------------------------------------------+\n");
        printf("\t* current \"vertex_id\" section size = %lld\n", sizeof(vertex_id_t));
        printf("\t* current slot size = %lld\n", sizeof(test_page_t::slot_t));
        printf("slotted_page::add_slot( <vertex id = 0x0A> ) = %s | %llu\n", result.first ? "true" : "false", result.second);
        print_line();
        if (result.first)
        {
            igraph::print_page<16>(*page);
            print_line();
            // Test slotted_page::add_adj_elems_for_small_page_unsafe()
            {
                printf("Dummy adjacency elements:\n");
                std::vector<test_page_t::adj_element_t> dummy_elements;
                for (int i = 1; i <= 5; ++i)
                {
                    dummy_elements.push_back(test_page_t::adj_element_t{ static_cast<adj_page_id_t>(i) /* adj_page_id */, static_cast<adj_offset_t>(10 + i) /* adj_offset */ });
                    printf("elem[%d] = [adj_page_id: 0x%02X] | [adj_offset: 0x%02X]\n", (i - 1), i, (10 + i));
                }
                printf("@ Add the elements of adjacency list to specific slot #0\n");
                printf("# Adjacency element representation\n\
\t+------------------------------------------------------------------------------------+\n\
\t| adj_page_id (user-def type) | adj_offset (user-def type) | payload (user-def type) |\n\
\t+------------------------------------------------------------------------------------+\n");
                printf("\t* current \"adj_page_id\" section size = %lld\n", sizeof(adj_page_id_t));
                printf("\t* current \"adj_offset\" section size  = %lld\n", sizeof(adj_offset_t));
                printf("\t* current adjacency element size = %lld\n", sizeof(test_page_t::adj_element_t));
                page->add_adj_elems_for_small_page_unsafe(0 /* slot offset */,
                                                          static_cast<adj_list_size_t>(dummy_elements.size()),
                                                          dummy_elements.data());
                printf("slotted_page::add_adj_elems_for_small_page_unsafe( <slot offset = 0>, <adj-elements = dummy> )\n");
                print_line();
                igraph::print_page<16>(*page);
                print_line();
            }
        }
    }

    // Test again slotted_page::is_empty()
    {
        bool result = page->is_empty();
        printf("slotted_page::is_empty() = %s\n", result ? "true" : "false");
        print_line();
    }

    // Test again slotted_page::number_of_slots()
    {
        printf("@ Get a number of slots stored in a slotted page\n");
        printf("slotted_page::number_of_slots() = %llu\n", page->number_of_slots());
        print_line();
    }

    // Test again slotted_page::scan()
    {
        auto result = page->scan();
        printf("@ Scan operation: scans free space in the slotted page, and returns result of a couple of informations as follows:\n\
(1) Whether it is possible to store new slot to this page or not (boolean)\n\
(2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)\n");
        printf("slotted_page::scan() = %s | %llu\n", result.first ? "true" : "false", result.second);
        print_line();
    }

    // Test again slotted_page::add_slot()
    {
        auto result = page->add_slot(0x0A /* vertex id */);
        printf("@ Add slot operation (slot #1): add a slot into slotted page, this operation returns,\n\
(1) Operation result (boolean)\n\
(2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)\n");
        printf("# Slot representation in the slotted page\n\
\t+--------------------------------------------------------+\n\
\t| vertex_id (user-def type) | internal_offset (uint32_t) |\n\
\t+--------------------------------------------------------+\n");
        printf("\t* current \"vertex_id\" section size = %lld\n", sizeof(vertex_id_t));
        printf("\t* current slot size = %lld\n", sizeof(test_page_t::slot_t));
        printf("slotted_page::add_slot( <vertex id = 0x0A> ) = %s | %llu\n", result.first ? "true" : "false", result.second);
        print_line();
        if (result.first)
        {
            igraph::print_page<16>(*page);
            print_line();
            // Test again slotted_page::add_adj_elems_for_small_page_unsafe()
            {
                printf("Dummy adjacency elements:\n");
                std::vector<test_page_t::adj_element_t> dummy_elements;
                for (int i = 1; i <= 5; ++i)
                {
                    dummy_elements.push_back(test_page_t::adj_element_t{ static_cast<adj_page_id_t>(i + 5) /* adj_page_id */, static_cast<adj_offset_t>(0x0F) /* adj_offset */ });
                    printf("elem[%d] = [adj_page_id: 0x%02X] | [adj_offset: 0x%02X]\n", (i - 1), i + 5, 0x0F);
                }
                printf("@ Add the elements of adjacency list to specific slot #1\n");
                printf("# Adjacency element representation\n\
\t+------------------------------------------------------------------------------------+\n\
\t| adj_page_id (user-def type) | adj_offset (user-def type) | payload (user-def type) |\n\
\t+------------------------------------------------------------------------------------+\n");
                printf("\t* current \"adj_page_id\" section size = %lld\n", sizeof(adj_page_id_t));
                printf("\t* current \"adj_offset\" section size  = %lld\n", sizeof(adj_offset_t));
                printf("\t* current \"adj_payload\" section size = %lld\n", test_page_t::adj_payload_size);
                printf("\t* current adjacency element size = %lld\n", sizeof(test_page_t::adj_element_t));
                page->add_adj_elems_for_small_page_unsafe(1 /* slot offset */,
                                                          static_cast<adj_list_size_t>(dummy_elements.size()),
                                                          dummy_elements.data());
                printf("slotted_page::add_adj_elems_for_small_page_unsafe( <slot offset = 1>, <adj-elements = dummy> )\n");
                print_line();
                igraph::print_page<16>(*page);
                print_line();
            }

            // Test again slotted_page::number_of_slots()
            {
                printf("@ Get a number of slots stored in a slotted page\n");
                printf("slotted_page::number_of_slots() = %llu\n", page->number_of_slots());
                print_line();
            }
        }

    }

    // Test again slotted_page::scan()
    {
        auto result = page->scan();
        printf("@ Scan operation: scans free space in the slotted page, and returns result of a couple of informations as follows:\n\
(1) Whether it is possible to store new slot to this page or not (boolean)\n\
(2) The number of storable adjacency list elements for new slot (if (1) is not, set to zero)\n");
        printf("slotted_page::scan() = %s | %llu\n", result.first ? "true" : "false", result.second);
        print_line();
    }
}

void test_large_page()
{
    print_line();
    puts("Large-page test");
    print_line();
    printf("# Current page size = %llu\n", PAGE_SIZE);
    printf("# Page footer size  = %llu\n", sizeof(test_page_t::footer_t));
    printf("# Data section size in the page = %llu (PAGE_SIZE - FOOTER_SIZE)\n", test_page_t::DATA_SECTION_SIZE);
    const size_t test_list_size = test_page_t::DATA_SECTION_SIZE;
    printf("# Test adjacency list size = %llu (DATA_SECTION_SIZE * 3)\n", test_list_size);
    const size_t storable_list_size = test_page_t::storable_list_size();
    const size_t storable_extended_list_size = test_page_t::storable_extended_list_size();
    printf("# The maximum number of adjacency list elements to store in first page = %llu\n", storable_list_size);
    printf("# The maximum number of adjacency list elements to store per extended page = %llu\n", storable_extended_list_size);
    const size_t number_of_pages = storable_list_size >= test_list_size ?
        1 :
        1 + static_cast<size_t>(ceil(static_cast<double>(test_list_size - storable_list_size) / storable_extended_list_size));
    printf("# The number of pages required = %llu pages\n", number_of_pages);
    print_line();

    // Generate dummy data
    std::vector<test_page_t::adj_element_t> dummy_elems;
    for (size_t i = 0; i < test_list_size; ++i)
    {
        dummy_elems.push_back(
            test_page_t::adj_element_t{
            static_cast<adj_page_id_t>(i),
            static_cast<adj_offset_t>(i)
        }
        );
    }

    printf("Dummy adjacency elements:\n");
    for (size_t i = 0; i < 5; ++i)
    {
        printf("elem[%llu] = [adj_page_id: 0x%02X] | [adj_offset: 0x%02X]\n",
               i,
               static_cast<uint32_t>(dummy_elems[i].adj_page_id),
               static_cast<uint32_t>(dummy_elems[i].adj_offset));
    }
    for (size_t i = 0; i < 3; ++i)
        printf("                        .\n");
    for (size_t i = test_list_size - 3; i < test_list_size; ++i)
    {
        printf("elem[%llu] = [adj_page_id: 0x%02X] | [adj_offset: 0x%02X]\n",
               i,
               static_cast<uint32_t>(dummy_elems[i].adj_page_id),
               static_cast<uint32_t>(dummy_elems[i].adj_offset));
    }
    print_line();

    printf("# Adjacency element representation\n\
\t+------------------------------------------------------------------------------------+\n\
\t| adj_page_id (user-def type) | adj_offset (user-def type) | payload (user-def type) |\n\
\t+------------------------------------------------------------------------------------+\n");
    printf("\t* current \"adj_page_id\" section size = %lld\n", sizeof(adj_page_id_t));
    printf("\t* current \"adj_offset\" section size  = %lld\n", sizeof(adj_offset_t));
    printf("\t* current adjacency element size = %lld\n", sizeof(test_page_t::adj_element_t));
    print_line();

    size_t offset = 0;

    // Create pages
    std::vector<test_page_t> pages;
    pages.resize(number_of_pages);
    pages[0].footer.flags |= igraph::page_flag::LARGE_PAGE | igraph::page_flag::LEAD_PAGE;
    for (size_t i = 1; i < number_of_pages; ++i)
    {
        pages[i].footer.flags |= igraph::page_flag::LARGE_PAGE | igraph::page_flag::EXTENDED_PAGE;
    }

    size_t remained_elems = test_list_size;

    // Fill the lead page
    const vertex_id_t dummy_vid = 0xFF;
    {
        const size_t push_size = (remained_elems >= storable_list_size) ? storable_list_size : remained_elems;
        pages[0].add_slot_unsafe(dummy_vid);
        pages[0].add_adj_elems_for_lead_page_unsafe(0 /* slot offset */,
                                                    test_list_size,
                                                    static_cast<adj_list_size_t>(push_size),
                                                    dummy_elems.data() + offset);
        remained_elems -= push_size;
        offset += push_size;
    }
    // Fill the extended pages
    for (size_t i = 1; i < (number_of_pages); ++i)
    {
        const size_t push_size = (remained_elems >= storable_extended_list_size) ? storable_extended_list_size : remained_elems;
        pages[i].add_extended_slot_unsafe(dummy_vid);
        pages[i].add_adj_elems_for_ext_page_unsafe(0 /* slot offset */,
                                                   static_cast<adj_list_size_t>(push_size),
                                                   dummy_elems.data() + offset);
        remained_elems -= push_size;
        offset += push_size;
    }

    printf("Test result %llu pages\n", number_of_pages);

    print_line();
    printf("[LEAD PAGE] The number of elements of adjacency list in the page: %llu\n",
           static_cast<size_t>(pages[0].local_adj_list_size(0)));
    igraph::print_page<16>(pages[0]);
    print_line();
    for (size_t i = 1; i < number_of_pages; ++i)
    {
        print_line();
        printf("[EXTENDED PAGE] The number of elements of adjacency list in the page: %llu\n",
               static_cast<size_t>(pages[i].local_adj_list_size(0)));
        igraph::print_page<16>(pages[i]);
        print_line();
    }
}