#include <infograph/pagedb.h>
#include <fstream>

namespace igraph {

pagedb_info::pagedb_info(const char* filepath)
{
    read_from_file(filepath);
}

bool pagedb_info::read_from_file(const char* filepath) noexcept
{
    try
    {
        std::ifstream ifs{ filepath, std::ios::in | std::ios::binary };
        ifs.read(reinterpret_cast<char*>(&num_pages), sizeof(decltype(this->num_pages)));
        return true;
    }
    catch (std::exception& e)
    {
        return false;
    }
}

} // !namespace igraph