#include <sstream>

#include "psql_parse/driver.hpp"

bool parseString(const std::string&& str) {
    std::istringstream iss(str);
    psql_parse::driver driver;
    return driver.parse(iss);
}