#ifndef _INFOGRAPH_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_
#define _INFOGRAPH_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_
#include <string>

namespace utility {

inline ::std::string trim_left(::std::string str)
{
    ::std::string::size_type n = str.find_first_not_of(" \t\v\n");
    return n == ::std::string::npos ? str : str.substr(n, str.length());
}

inline ::std::string trim_right(::std::string str)
{
    ::std::string::size_type n = str.find_last_not_of(" \t\v\n");
    return n == ::std::string::npos ? str : str.substr(0, n + 1);
}

inline std::string trim(::std::string str)
{
    return trim_left(trim_right(str));
}

} // !namespace utility

#endif // !_INFOGRAPH_SAMPLES_PAGEDB_GENERATOR_UTILITY_H_
