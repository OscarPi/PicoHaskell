#ifndef PICOHASKELL_GENERATION_HPP
#define PICOHASKELL_GENERATION_HPP

#include <memory>
#include <ostream>
#include "stg/stg.hpp"

void generate_target_code(const std::unique_ptr<STGProgram> &program, std::ostream &output);

#endif //PICOHASKELL_GENERATION_HPP
