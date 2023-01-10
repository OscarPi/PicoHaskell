#include <iostream>
#include <fstream>
#include <memory>
#include "parser/location.hh"
#include "parser/syntax.hpp"
#include "prelude/prelude.hpp"
#include "parser/parser.hpp"
#include "lexer/yylex.hpp"
#include "lexer/lexer.hpp"
#include "types/type_check.hpp"
#include "stg/stg.hpp"
#include "generation/generation.hpp"

void print_usage_message(std::ostream &s) {
    s << "Usage: picohaskell [-i <input file>] [-o <output file>]" << std::endl;
    s << "If no input file is specified, stdin will be used." << std::endl;
    s << "If no output file is specified, stdout will be used." << std::endl;
}

int main (int argc, char *argv[]) {
    FILE *input = stdin;
    std::ofstream output_file;
    std::ostream *output = &std::cout;

    for (int i = 1; i < argc; ) {
        if (strcmp(argv[i], "-h") == 0) {
            print_usage_message(std::cout);
            return 0;
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i+1 < argc) {
                input = fopen(argv[i+1], "r");
                if (input == NULL) {
                    std::cerr << "Could not open input file." << std::endl;
                    return 1;
                }
                i += 2;
            } else {
                print_usage_message(std::cerr);
                return 1;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i+1 < argc) {
                output_file.open(argv[i+1]);
                if (!output_file) {
                    std::cerr << "Could not open output file." << std::endl;
                    return 1;
                }
                output = &output_file;
                i += 2;
            } else {
                print_usage_message(std::cerr);
                return 1;
            }
        } else {
            print_usage_message(std::cerr);
            return 1;
        }
    }

    std::unique_ptr<Program> program = std::make_unique<Program>();
    add_prelude(program.get());
    yy::location loc;
    yyin = input;
    YY_BUFFER_STATE buffer = yy_create_buffer(yyin, YY_BUF_SIZE);
    yy_switch_to_buffer(buffer);
    reset_start_condition();
    yy::parser parse(loc, program.get());
    parse.set_debug_level(false);
    int result = parse();
    yy_delete_buffer(buffer);
    if (result != 0) {
        std::cerr << "Parse error." << std::endl;
        return 1;
    }
    try {
        type_check(program, true);
    } catch (const TypeError &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Type inference failed." << std::endl;
        return 1;
    }
    std::unique_ptr<STGProgram> translated = translate(program);
    generate_target_code(translated, *output);

    output_file.close();
    fclose(input);
    return 0;
}
