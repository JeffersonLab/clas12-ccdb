#ifndef CLAS12_CCDB_PARSE_TIMESTAMP_HPP
#define CLAS12_CCDB_PARSE_TIMESTAMP_HPP

#include <ctime>
#include <string>

namespace clas12
{
namespace ccdb
{

std::time_t parse_timestamp(const std::string& input);

} // namespace clas12::ccdb
} // namespace clas12

#endif // CLAS12_CCDB_PARSE_TIMESTAMP_HPP
