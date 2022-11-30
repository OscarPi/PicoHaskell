#include <gtest/gtest.h>
#include <memory>
#include "parser/driver.hpp"
#include "lexer/lexer.hpp"

void reset_start_condition();

int parse_string(const char* str, std::shared_ptr<SyntaxTreeNode> *tree) {
    Driver drv;
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(drv, tree);
    int result = parse();
    yy_delete_buffer(buffer);
    return result;
}

TEST(Parser, ExampleTest) {
    std::shared_ptr<SyntaxTreeNode> tree;
    int result = parse_string("\\", &tree);
    ASSERT_EQ(result, 0);
}
