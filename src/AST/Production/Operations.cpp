#include "../include/Operations.hpp"
#include <memory>

namespace AST
{
    const std::map<std::string, std::unique_ptr<const OpDefinition>> OpRegistry::map = [] {
        std::map<std::string, std::unique_ptr<const OpDefinition>> operations;
        operations.emplace("+", std::make_unique<const Add>());
        operations.emplace("-", std::make_unique<const Subtract>());
        operations.emplace("unary-", std::make_unique<const Negate>());
        operations.emplace("*", std::make_unique<const Multiply>());
        operations.emplace("[implicit*]", std::make_unique<const Multiply>(true));
        operations.emplace("/", std::make_unique<const Divide>());
        operations.emplace("^", std::make_unique<const Exponentiate>());
        return operations;
    }();
};
