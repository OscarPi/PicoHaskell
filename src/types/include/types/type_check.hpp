#ifndef PICOHASKELL_TYPE_CHECK_HPP
#define PICOHASKELL_TYPE_CHECK_HPP
#include <memory>
#include "types/types.hpp"

void type_check(const std::unique_ptr<Program> &program);
std::vector<std::vector<std::string>> dependency_analysis(
        std::vector<std::string> names,
        const std::map<std::string, std::set<std::string>> &dependencies);
std::set<std::string> find_free_variables(const std::unique_ptr<Expression> &exp);

#endif //PICOHASKELL_TYPE_CHECK_HPP
