#include "prelude/prelude.hpp"
#include "lexer/yylex.hpp"
#include "lexer/lexer.hpp"

const char *prelude = R"##(
data Bool = True | False
--;
--(&&) :: Bool -> Bool -> Bool
--;
--(&&) a b = case a of { False -> False ; True -> b }
--;
--(||) :: Bool -> Bool -> Bool
--;
--(||) a b = case a of { True -> True ; False -> b }
--;
--not :: Bool -> Bool
--;
--not b = case b of { True -> False ; False -> True }
--;
--(/=) :: Int -> Int -> Bool
--;
--(/=) a b = not (a == b)
--;
--(/=.) :: Char -> Char -> Bool
--;
--(/=.) a b = not (a ==. b)
--;
--(.) :: (b -> c) -> (a -> b) -> a -> c
--;
--(.) f g = \x -> f (g x)
--;
--(++) :: [a] -> [a] -> [a]
--;
--(++) a b = case a of { [] -> b ; (x:xs) -> x : (xs ++ b) }
--;
--error :: [Char] -> a
--;
--error msg = error msg
--;
--case_error = error "Non-exhaustive patterns in case"
)##";

void bind_built_in_op(Program *program, const std::string &function_name, builtinop op) {
    program->add_named_function(
            0,
            function_name,
            {"a", "b"},
            new BuiltInOp(
                    0,
                    new Variable(0, "a"),
                    new Variable(0, "b"),
                    op));
}

void add_prelude(Program *program) {
    program->add_type_constructor(0, "Int", {}, {});
    program->add_type_constructor(0, "Char", {}, {});
    program->add_type_constructor(0, "->", {"a", "b"}, {});
    program->add_type_constructor(
            0,
            "()",
            {},
            {new DConstructor(0, "()", {})});
    program->add_type_constructor(
            0,
            "[]",
            {"a"},
            {
                    new DConstructor(0, "[]", {}),
                    new DConstructor(
                            0,
                            ":",
                            {
                                    new UniversallyQuantifiedVariable("a"),
                                    new TypeApplication(new TypeConstructor("[]"),
                                                        new UniversallyQuantifiedVariable("a"))})});
    for (int i = 1; i < 15; i++) {
        const std::string name = "(" + std::string(i, ',') + ")";
        std::vector<std::string> argument_variables;
        std::vector<Type*> types;
        for (int j = 0; j < i+1; j++) {
            argument_variables.push_back(std::to_string(j));
            types.push_back(new UniversallyQuantifiedVariable(std::to_string(j)));
        }
        program->add_type_constructor(
                0,
                name,
                argument_variables,
                {new DConstructor(0, name, types)});
    }

    //bind_built_in_op(program, "+", builtinop::add);
    //bind_built_in_op(program, "-", builtinop::subtract);
    //bind_built_in_op(program, "*", builtinop::times);
    //bind_built_in_op(program, "/", builtinop::divide);
    //bind_built_in_op(program, "==", builtinop::intequality);
    //bind_built_in_op(program, "==.", builtinop::charequality);
    //bind_built_in_op(program, "<", builtinop::lt);
    //bind_built_in_op(program, "<=", builtinop::lte);
    //bind_built_in_op(program, ">", builtinop::gt);
    //bind_built_in_op(program, ">=", builtinop::gte);

    yy::location loc;
    YY_BUFFER_STATE buffer = yy_scan_string(prelude);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(loc, program);
    parse();
    yy_delete_buffer(buffer);
}
