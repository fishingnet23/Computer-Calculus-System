#include "Operations.hpp"
#include <memory>

namespace AST
{
    const std::map<std::string, std::shared_ptr<OpDefinition>> OpRegistry::map = [] {
        std::map<std::string, std::shared_ptr<OpDefinition>> operations;
        operations.emplace("+", std::make_shared< Add>());
        operations.emplace("*", std::make_shared< Multiply>());
        operations.emplace("[implicit*]", std::make_shared< Multiply>(true));
        operations.emplace("^", std::make_shared< Exponentiate>());
        operations.emplace("log",std::make_shared<Logarithm>());

        operations.emplace("sin",std::make_shared<Sin>());
        operations.emplace("cos",std::make_shared<Cos>());

        operations.emplace("max",std::make_shared<Max>());
        operations.emplace("min",std::make_shared<Min>());

        operations.emplace("derivative",std::make_shared<Derivative>());
        return operations;
    }();

    const std::map<std::string, std::shared_ptr< SyntaxFluff>> OpRegistry::fluffyMap = [] {
        std::map<std::string, std::shared_ptr< SyntaxFluff>> operations;
        operations.emplace("-", std::make_shared<Subtract>());
        operations.emplace("unary-", std::make_shared< Negate>());
        operations.emplace("/", std::make_shared< Divide>());
        operations.emplace("ln",std::make_shared<NaturalLog>());
        operations.emplace("exp",std::make_shared<Exp>());
        operations.emplace("sqrt",std::make_shared<SquareRoot>());
        // operations.emplace("root",std::make_shared<NthRoot>());

        return operations;
    }();
};
