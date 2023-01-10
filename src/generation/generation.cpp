#include "generation/generation.hpp"

void generate_target_code(const std::unique_ptr<STGProgram> &program, std::ostream &output) {
    output << ".thumb_func" << std::endl;
    output << ".global run" << std::endl;
    output << "run:" << std::endl;
    output << "    B .test$label" << std::endl;
    output << ".test$label:" << std::endl;
    output << "    LDR R0, =helloworld" << std::endl;
    output << "    BL printf" << std::endl;
    output << "    BX LR" << std::endl;
    output << ".data" << std::endl;
    output << "            .align 4" << std::endl;
    output << "helloworld: .asciz \"Hello, world!\\n\"" << std::endl;
}
