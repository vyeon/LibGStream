#include <infograph/type/generic/pagedb_generator.h>
#include <string>
#include <sstream>

int main()
{
    // Put an input file name here
    // It is assumed that the input file is sorted by VID.
    std::ifstream ifs{ "test_dataset.txt" };

    /* The following code defines the "Slotted Page" type corresponding to the PageDB what you want to create. 
     * For more information about Slotted Page, see the "SlottedPageTutorial" example.
     */
    using vertex_id_t = uint32_t;
    using adj_page_id_t = uint16_t;
    using adj_slot_offset_t = uint16_t;
    using adj_list_size_t = uint32_t;
    using payload_t = void;
    constexpr size_t PAGE_SIZE = 56;
    using page_t = igraph::slotted_page<vertex_id_t, adj_page_id_t, adj_slot_offset_t, adj_list_size_t, PAGE_SIZE, payload_t>;

    /* To create a PageDB, you should to create an edge list of each vertex from input file.
     * The following code describes how to create the edge list from the input file.
     * Creating logic is depending on the format of the input data.
     */
    using adj_element_t = page_t::adj_element_t;
    
    auto cvt_edges = [](std::ifstream& ifs, std::vector<adj_element_t>& out_vec) -> void
    {
        out_vec.clear();
        std::string buffer;
        while (std::getline(ifs, buffer))
        {
            std::istringstream iss{ buffer };
            std::string token;
        }
    };

    std::vector<adj_element_t> adj_elems;
       
}
