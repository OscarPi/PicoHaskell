#include <algorithm>
#include <sstream>
#include "generation/generation.hpp"

std::string sanitise_name(std::string name) {
    if (name == ".") {
        return ".compose";
    } else if (name == "+") {
        return ".plus";
    } else if (name == "-") {
        return ".subtract";
    } else if (name == "*") {
        return ".multiply";
    } else if (name == "/") {
        return ".divide";
    } else if (name == "==") {
        return ".equality";
    } else if (name == "==.") {
        return ".charequality";
    } else if (name == "/=") {
        return ".inequality";
    } else if (name == "/=.") {
        return ".charinequality";
    } else if (name == "<") {
        return ".lt";
    } else if (name == "<=") {
        return ".lte";
    } else if (name == ">") {
        return ".gt";
    } else if (name == ">=") {
        return ".gte";
    } else if (name == "&&") {
        return ".land";
    } else if (name == "||") {
        return ".lor";
    } else if (name == "++") {
        return ".concatenate";
    } else if (name == "[]") {
        return ".Nil";
    } else if (name == ":") {
        return ".Cons";
    } else if (name == "()") {
        return ".Unit";
    } else if (name[0] == '(') {
        return ".Tuple" + std::to_string(name.size() - 1);
    }

    std::replace(name.begin(), name.end(), '\'', '$');
    return name;
}

void generate_standard_constructor_info_tables_and_closures(
        const std::unique_ptr<STGProgram> &program,
        std::ostream &output) {
    for (const auto &[name, constructor]: program->data_constructors) {
        output << ".align 4 @ info table for " << name << std::endl;
        output << ".word 0 @ evacuation code" << std::endl;
        output << ".word 0 @ scavenge code" << std::endl;
        output << ".thumb_func" << std::endl;
        output << sanitise_name(name) << "_standard_entry_code:" << std::endl;
        output << "    ADD R1, #4" << std::endl;
        output << "    LDR R6, [R1] @ pop return address from B stack to R6" << std::endl;
        output << "    MOVS R5, #" << constructor.tag << " @ put tag in R5" << std::endl;
        output << "    BX R6 @ jump to return address" << std::endl;

        if (constructor.arity == 0) {
            output << ".align 4" << std::endl;
            output << sanitise_name(name) << "_closure:" << std::endl;
            output << ".word " << sanitise_name(name) << "_standard_entry_code @ info pointer" << std::endl;
        }
    }

    output << ".align 4 @ info table for literals" << std::endl;
    output << ".word 0 @ evacuation code" << std::endl;
    output << ".word 0 @ scavenge code" << std::endl;
    output << ".thumb_func" << std::endl;
    output << ".literal_standard_entry_code:" << std::endl;
    output << "    ADD R1, #4" << std::endl;
    output << "    LDR R6, [R1] @ pop return address from B stack to R6" << std::endl;
    output << "    BX R6 @ jump to return address" << std::endl;
}

void generate_info_table(
        const std::string &name,
        const std::unique_ptr<STGLambdaForm> &lambda_form,
        std::ostream &output) {
    output << ".align 4 @ info table for " << name << std::endl;
    output << ".word 0 @ evacuation code" << std::endl;
    output << ".word 0 @ scavenge code" << std::endl;
    output << sanitise_name(name) << "_standard_entry_code:" << std::endl;

    //ARGUMENT SATISFACTION CHECK

    //STACK OVERFLOW CHECK

    //HEAP OVERFLOW CHECK

    //INFO POINTER UPDATE

    //UPDATE FRAME CONSTRUCTION

    switch (lambda_form->expr->get_form()) {

    }
}

void generate_code_for_bindings(const std::unique_ptr<STGProgram> &program, std::ostream &output) {
    std::stringstream data_section;

    data_section << ".align 4" << std::endl;
    data_section << ".data" << std::endl;

    for (const auto &[name, lambda_form]: program->bindings) {
        if (lambda_form->argument_variables.empty() && lambda_form->expr->get_form() == stgform::constructor) {
            auto constructor = dynamic_cast<STGConstructor *>(lambda_form->expr.get());
            if (constructor->arguments.empty()) {
                output << sanitise_name(name) << "_closure = "
                       << sanitise_name(constructor->constructor_name) << "_closure" << std::endl;
            } else {
                output << ".align 4" << std::endl;
                output << sanitise_name(name) << "_closure:" << std::endl;
                output << ".word " << sanitise_name(constructor->constructor_name)
                       << "_standard_entry_code @ info pointer" << std::endl;
                for (const std::string &arg: constructor->arguments) {
                    output << ".word " << sanitise_name(arg) << "_closure @ arg" << std::endl;
                }
            }
        } else if (lambda_form->argument_variables.empty() && lambda_form->expr->get_form() == stgform::literal){
            auto literal = dynamic_cast<STGLiteral *>(lambda_form->expr.get());
            output << ".align 4" << std::endl;
            output << sanitise_name(name) << "_closure:" << std::endl;
            output << ".word .literal_standard_entry_code @ info pointer" << std::endl;
            if (std::holds_alternative<int>(literal->value)) {
                output << ".word " << std::get<int>(literal->value) << std::endl;
            } else if (std::holds_alternative<char>(literal->value)) {
                output << ".word " << ((int) std::get<char>(literal->value)) << std::endl;
            }
        } else {
            generate_info_table(name, lambda_form, output);
            if (lambda_form->updatable) {
                data_section << sanitise_name(name) << "_closure:" << std::endl;
                data_section << ".word " << sanitise_name(name) << "_standard_entry_code @ info pointer" << std::endl;
                data_section << ".word 0 @ empty word to hold indirection" << std::endl;
            } else {
                output << sanitise_name(name) << "_closure:" << std::endl;
                output << ".word " << sanitise_name(name) << "_standard_entry_code @ info pointer" << std::endl;
            }
        }
    }

    output << data_section.str() << std::endl;
}

void generate_target_code(const std::unique_ptr<STGProgram> &program, std::ostream &output) {
    output << ".thumb_func" << std::endl;
    output << ".global run" << std::endl;
    output << "run:" << std::endl;
    output << "    PUSH {R4, R5, R6, R7, LR} @ save registers" << std::endl;
    output << "    MOV R4, R8" << std::endl;
    output << "    MOV R5, R9" << std::endl;
    output << "    MOV R6, R10" << std::endl;
    output << "    MOV R7, R11" << std::endl;
    output << "    PUSH {R4, R5, R6, R7} @ we will use lots of registers" << std::endl;
    output << "    MOV R8, R0" << std::endl;
    output << "    MOV R9, R1 @ initialise stack base registers" << std::endl;
    output << "    LDR R6, =.main_return" << std::endl;
    output << "    STR R6, [R1]" << std::endl;
    output << "    SUB R1, #4 @ push return address on B stack" << std::endl;
    output << "    LDR R4, =main_closure @ put address of main closure in Node register" << std::endl;
    output << "    LDR R5, [R4] @ load address of standard entry code into R5" << std::endl;
    output << "    BX R5 @ jump to standard entry code" << std::endl;
    output << ".thumb_func" << std::endl;
    output << ".main_return:" << std::endl;
    output << "    CMP R5, #0 @ inspect returned tag" << std::endl;
    output << "    BEQ .handle_nil" << std::endl;
    output << "    LDR R6, [R4, #8] @ load address of tail closure into R6" << std::endl;
    output << "    STR R6, [R0]" << std::endl;
    output << "    ADD R0, #4 @ push tail closure onto A stack so it is preserved" << std::endl;
    output << "    LDR R6, =.char_return" << std::endl;
    output << "    STR R6, [R1]" << std::endl;
    output << "    SUB R1, #4 @ push return address on B stack" << std::endl;
    output << "    LDR R4, [R4, #4] @ load address of char closure into Node register" << std::endl;
    output << "    LDR R5, [R4] @ load address of standard entry code into R5" << std::endl;
    output << "    BX R5 @ jump to standard entry code" << std::endl;
    output << ".thumb_func" << std::endl;
    output << ".char_return:" << std::endl;
    output << "    PUSH {R0, R1, R2, R3} @ save registers" << std::endl;
    output << "    LDR R0, [R4, #4] @ put char in R0 (first argument to putchar)" << std::endl;
    output << "    BL putchar" << std::endl;
    output << "    POP {R0, R1, R2, R3} @ restore registers" << std::endl;
    output << "    LDR R6, =.main_return @ push return address on B stack" << std::endl;
    output << "    STR R6, [R1]" << std::endl;
    output << "    SUB R1, #4" << std::endl;
    output << "    SUB R0, #4" << std::endl;
    output << "    LDR R4, [R0] @ pop address of tail closure from A stack to Node register" << std::endl;
    output << "    LDR R5, [R4] @ load address of standard entry code into R5" << std::endl;
    output << "    BX R5 @ jump to standard entry code" << std::endl;
    output << ".handle_nil:" << std::endl;
    output << "    POP {R4, R5, R6, R7} @ restore some registers" << std::endl;
    output << "    MOV R8, R4" << std::endl;
    output << "    MOV R9, R5" << std::endl;
    output << "    MOV R10, R6" << std::endl;
    output << "    MOV R11, R7" << std::endl;
    output << "    POP {R4, R5, R6, R7, PC} @ restore final registers and return" << std::endl;
    output << ".ltorg @ tell the assembler to stick a literal pool here" << std::endl;
    generate_standard_constructor_info_tables_and_closures(program, output);
    generate_code_for_bindings(program, output);
};
