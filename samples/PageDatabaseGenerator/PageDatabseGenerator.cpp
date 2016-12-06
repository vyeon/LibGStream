#include <infograph/type/generic/pagedb_generator.h>
#include <string>
#include <sstream>

/* The following code defines the "Slotted Page" type corresponding to the PageDB what you want to create.
 * For more information about Slotted Page, see the "SlottedPageTutorial" example. */
using vertex_id_t = uint32_t;
using adj_page_id_t = uint16_t;
using adj_slot_offset_t = uint16_t;
using adj_list_size_t = uint32_t;
using payload_t = void;
constexpr size_t PAGE_SIZE = 56;
using page_t = igraph::slotted_page<vertex_id_t, adj_page_id_t, adj_slot_offset_t, adj_list_size_t, PAGE_SIZE, payload_t>;

/* The following code describes how to create the edge list from the input file. */
using adj_element_t = page_t::adj_element_t;
using rid_table_generator_t = igraph::rid_table_generator<page_t, std::vector>;
using rid_tuple_t = rid_table_generator_t; // same as page_t::rid_tuple_t
using rid_table_t = rid_table_generator_t::rid_table_t;
using db_generator_t = igraph::db_generator<page_t, rid_table_t>;
using edge_t = db_generator_t::edge_t;

/* Declare function prototypes */
size_t convert_edges_per_vertex(vertex_id_t base_vid, std::ifstream& ifs, std::vector<edge_t>& out_vec);
rid_table_t generate_rid_table(vertex_id_t start_vid, std::ifstream& ifs);

int main()
{
    // Put an input file name here
    // It is assumed that the input file is sorted by VID.
    std::ifstream ifs{ "test_dataset.txt" };

    rid_table_t table = generate_rid_table(0, ifs);
}

/* To create a PageDB, you should to create an edge list of each vertex from input file.
 * Creating logic is depending on the format of the input data.
 * 
 * This function to generate an edge list from ifstream 
 * @param   base_vid: base vertex ID
 * @param   ifs: destination input file stream
 * @param   out_vec: a result vector of edges (as a result edge list)
 * @returns the number of edges in the result edge list */
size_t convert_edges_per_vertex(vertex_id_t base_vid, std::ifstream& ifs, std::vector<edge_t>& out_vec)
{
    out_vec.clear(); // Clear a result vector before working
    std::string buffer;
    edge_t edge;
    edge.src = base_vid;

    auto old_pos = ifs.tellg();
    while (std::getline(ifs, buffer))
    {
        std::istringstream iss{ buffer };
        std::string token;
        std::getline(iss, token, '\t');

        vertex_id_t src = std::stoul(token);
        if (src != base_vid)
        {
            /* Break a loop when the source vertex ID changes */
            ifs.seekg(old_pos); // rewind the last fpos of ifstream before break the loop
            break;
        }

        std::getline(iss, token, '\t');
        edge.dst = std::stoul(token);
        printf("%u\t-> %u\t", edge.src, edge.dst);
        out_vec.push_back(edge);
    }

    return out_vec.size();
}

rid_table_t generate_rid_table(vertex_id_t start_vid, std::ifstream& ifs)
{
    rid_table_generator_t generator;
   
    vertex_id_t& vid = start_vid;
    std::vector<edge_t> edge_list; // an edge list per vertex
    while (!ifs.eof())
    {
        auto num_edges = convert_edges_per_vertex(vid, ifs, edge_list);
        if (num_edges == 0)
        {
            ++vid;
            continue;
        }
        generator.iteration_per_vertex(vid, num_edges);
    }
    generator.flush();
    
    return generator.table;
}