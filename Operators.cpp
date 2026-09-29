#include "Operators.hpp"

OperationRegistry::OperationRegistry()
{
    // binary operators
    procedureRegistry["+"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
        return args[0] + args[1];
        };
    procedureRegistry["-"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
        return args[0] - args[1];
        };
    procedureRegistry["*"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        return args[0] * args[1];
        };
    procedureRegistry["^"] = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        return std::pow(args[0], args[1]);
        };

    // unary operators
    procedureRegistry["sin"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        };
    procedureRegistry["cos"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument");
        return std::cos(args[0]);
        };
    procedureRegistry["ln"] = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument");
        return std::log(args[0]);
        };

}

double OperationRegistry::execute(const std::string& op, const std::vector<double>& args) const
{
    auto it = procedureRegistry.find(op);
    if (it == procedureRegistry.end()) {
        throw std::runtime_error("Unknown operator: " + op);
    }
    return it->second(args); // call the registered function
}
