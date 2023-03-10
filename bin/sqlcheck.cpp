#include "psql_parse/driver.hpp"
#include "psql_parse/visit.hpp"

int main() {
    psql_parse::driver driver_;
    bool success = driver_.parse(std::cin);
    if (success) {
        psql_parse::printer printer_{std::cout};
        printer_.print(driver_.getResult());
        std::cout << std::endl;
    }
}