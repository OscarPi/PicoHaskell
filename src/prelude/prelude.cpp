#include "prelude/prelude.hpp"
#include "lexer/yylex.hpp"
#include "lexer/lexer.hpp"

const char *prelude = R"##(
data Bool = True | False
;
(&&) :: Bool -> Bool -> Bool
;
(&&) a b = case a of { False -> False ; True -> b }
;
(||) :: Bool -> Bool -> Bool
;
(||) a b = case a of { True -> True ; False -> b }
)##";

void add_prelude(Program *program) {
    yy::location loc;
    YY_BUFFER_STATE buffer = yy_scan_string(prelude);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(loc, program);
    parse();
    yy_delete_buffer(buffer);
}
