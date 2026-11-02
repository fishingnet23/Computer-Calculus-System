#include "Operations.hpp"


namespace AST
{
    void OpRegistry::initialize() {
        static bool initialized = false;
        if (initialized) return; // Prevent double-execution
        initialized = true;

        auto& ops = getMutableMap();
        ops.emplace("+", std::make_shared<Add>());
        ops.emplace("*", std::make_shared<Multiply>());
        ops.emplace("^", std::make_shared<Exponentiate>());
        ops.emplace("log", std::make_shared<Logarithm>());
        ops.emplace("sin", std::make_shared<Sin>());
        ops.emplace("cos", std::make_shared<Cos>());
        ops.emplace("max", std::make_shared<Max>());
        ops.emplace("min", std::make_shared<Min>());
        ops.emplace("derivative", std::make_shared<Derivative>());

        auto& fluff = getMutableFluffyMap();
        fluff.emplace("-", std::make_shared<Subtract>());
        fluff.emplace("[implicit*]", std::make_shared<ImplicitMul>());
        fluff.emplace("unary-", std::make_shared<Negate>());
        fluff.emplace("/", std::make_shared<Divide>());
        fluff.emplace("ln", std::make_shared<NaturalLog>());
        fluff.emplace("exp", std::make_shared<Exp>());
        fluff.emplace("sqrt", std::make_shared<SquareRoot>());
    }
}

    