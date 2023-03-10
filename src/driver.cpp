#include "psql_parse/driver.hpp"

psql_parse::driver::driver()
: scanner_(nullptr)
, trace_scanning_(false)
, trace_parsing_(false)
, scanner_err_(std::cerr)
, result_() { }

bool psql_parse::driver::parse(std::istream &in) {
    if (scanner_ == nullptr) {
        scanner_ = std::make_unique<scanner>(&in);
    } else {
        scanner_->reset();
        scanner_->switch_streams(in, scanner_err_);
    }
    scanner_->set_debug(trace_scanning_);
    parser p(*this);
    p.set_debug_level(trace_parsing_);
	nf.clear();
    return p.parse() == 0;
}

[[maybe_unused]] void psql_parse::driver::error(const psql_parse::location &loc, const std::string &message) {
    std::cerr << loc << " " << message << std::endl;
}

psql_parse::Statement& psql_parse::driver::getResult() {
    return result_;
}
