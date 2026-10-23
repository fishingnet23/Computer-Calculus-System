#include "../include/Operations.hpp"
#include <memory>

namespace AST
{
    const std::map<std::string, std::shared_ptr<OpDefinition>> OpRegistry::map = [] {
        std::map<std::string, std::shared_ptr<OpDefinition>> operations;
        operations.emplace("+", std::make_shared< Add>());
        operations.emplace("*", std::make_shared< Multiply>());
        operations.emplace("[implicit*]", std::make_shared< Multiply>(true));
        operations.emplace("^", std::make_shared< Exponentiate>());
        return operations;
    }();

    const std::map<std::string, std::shared_ptr< SyntaxFluff>> OpRegistry::fluffyMap = [] {
        std::map<std::string, std::shared_ptr< SyntaxFluff>> operations;
        operations.emplace("-", std::make_shared< Subtract>());
        operations.emplace("unary-", std::make_shared< Negate>());
        operations.emplace("/", std::make_shared< Divide>());
        return operations;
    }();
};
