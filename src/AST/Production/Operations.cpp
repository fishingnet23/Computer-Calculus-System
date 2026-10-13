#include "../include/Operations.hpp"

namespace AST
{
    const std::map<std::string, std::shared_ptr<const OpDefinition>> OpRegistry::map = 
    {
        {"+", std::make_shared<Add>()},
        {"-", std::make_shared<Subtract>()},
        {"unary-", std::make_shared<Negate>()},
        {"*", std::make_shared<Multiply>()}, // explicit
        {"[implicit*]",std::make_shared<Multiply>(true)}, // implicit
        {"/", std::make_shared<Divide>()},
        {"^", std::make_shared<Exponentiate>()}


    };
};
