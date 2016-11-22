#include <infograph/type/generic/pagedb.h>
#include <fstream>

namespace igraph {

pagedb_info::pagedb_info(const char* filepath)
{
    read_from_file(filepath);
}

void pagedb_info::read_from_file(const char* filepath) 
{
    /* Attention: Please refer to "@ Slotted Page Database Information File Representation" in the pagedb.h before modification. */
    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };
    ifs.read(reinterpret_cast<char*>(&num_pages), StringBufferSize);
    ifs.read(reinterpret_cast<char*>(&num_pages), sizeof(decltype(num_pages)));
}

} // !namespace igraph