#ifndef PICOHASKELL_TYPE_CHECK_HPP
#define PICOHASKELL_TYPE_CHECK_HPP
#include <memory>
#include "types/types.hpp"

void type_check(const std::unique_ptr<Program> &program);

#endif //PICOHASKELL_TYPE_CHECK_HPP
