#include <iostream>

#include "psql_parse/driver.hpp"
#include <sstream>

int main() {

    psql_parse::driver driver;

    auto testString = "192 2910x 30291";
    auto iss = std::istringstream(testString);

    driver.parse(iss);

    std::cout << driver.result_.size() << std::endl;

}
