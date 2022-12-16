#include "parser/driver.hpp"
#include "parser/parser.hpp"
#include "parser/syntax.hpp"
#include <memory>

Driver::Driver () : trace_parsing (false), trace_scanning (false) {
    variables["one"] = 1;
    variables["two"] = 2;
}

int Driver::parse (const std::string &f)
{
    file = f;
    location.initialize(&file);
    scan_begin();
    std::unique_ptr<Program> tree;
    yy::parser parse(*this, tree.get());
    parse.set_debug_level(trace_parsing);
    int res = parse();
    scan_end();
    return res;
}
