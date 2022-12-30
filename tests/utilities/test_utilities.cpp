#include <vector>
#include "test/test_utilities.hpp"
#include "parser/parser.hpp"
#include "parser/driver.hpp"
#include "lexer/lexer.hpp"


std::vector<yy::parser::symbol_type> lex_string(const char* str) {
    std::vector<yy::parser::symbol_type> result;
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    while (true) {
        result.push_back(yylex(drv));
        if (result.back().kind() == yy::parser::symbol_kind_type::S_YYEOF) {
            result.pop_back();
            break;
        }
    }
    yy_delete_buffer(buffer);
    return result;
}

int parse_string(const char* str, Program *program) {
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(drv, program);
    int result = parse();
    yy_delete_buffer(buffer);
    return result;
}

bool same_type(const Type *a, const Type *b) {
    if (a->get_form() != b->get_form()) {
        return false;
    }
    switch (a->get_form()) {
        case typeform::universallyquantifiedvariable:
            return dynamic_cast<const UniversallyQuantifiedVariable*>(a)->id ==
                   dynamic_cast<const UniversallyQuantifiedVariable*>(b)->id;
        case typeform::constructor:
            return dynamic_cast<const TypeConstructor*>(a)->id == dynamic_cast<const TypeConstructor*>(b)->id;
        case typeform::application:
            return same_type(
                    dynamic_cast<const TypeApplication*>(a)->left.get(),
                    dynamic_cast<const TypeApplication*>(b)->left.get()) &&
                   same_type(
                           dynamic_cast<const TypeApplication*>(a)->right.get(),
                           dynamic_cast<const TypeApplication*>(b)->right.get());
    }
}
