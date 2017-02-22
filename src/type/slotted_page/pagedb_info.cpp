#include <infograph/type/slotted_page/pagedb.h>

namespace igraph {

//pagedb_info::pagedb_info(const char* filepath)
//{
//    read_from_file(filepath);
//}
//
//void pagedb_info::read_from_file(const char* filepath)
//{
//    std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };
//    read_from_stream(ifs);
//}
//
//void pagedb_info::read_from_stream(std::ifstream& ifs)
//{
//    /* Attention: Please refer to "@ Slotted Page Database Information File Representation" in the pagedb.h before modification. */
//    ifs.read(reinterpret_cast<char*>(&num_pages), StringBufferSize);
//    ifs.read(reinterpret_cast<char*>(&num_pages), sizeof(decltype(num_pages)));
//}
//
//void pagedb_info::write_to_stream(std::ofstream& ofs)
//{
//    /* Attention: Please refer to "@ Slotted Page Database Information File Representation" in the pagedb.h before modification. */
//    ofs.write(reinterpret_cast<char*>(&num_pages), StringBufferSize);
//    ofs.write(reinterpret_cast<char*>(&num_pages), sizeof(decltype(num_pages)));
//}

} // !namespace igraph
