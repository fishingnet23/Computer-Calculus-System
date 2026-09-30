#include "Operators.hpp"
namespace AST
{
OperationRegistry::OperationRegistry()
{
    // binary operators
    registry["+"] = Operation(1,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
        return args[0] + args[1];
        });
    registry["-"] = Operation(1,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
        return args[0] - args[1];
        });
    registry["*"] = Operation(2,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        return args[0] * args[1];
        });
    registry["^"] = Operation(3,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        return std::pow(args[0], args[1]);
        });

    // unary operators
    registry["sin"] = Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        },Operation::Format::PREFIX);
    registry["cos"] = Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument");
        return std::cos(args[0]);
        }, Operation::Format::PREFIX);
    registry["ln"] = Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument");
        return std::log(args[0]);
        }, Operation::Format::PREFIX);

}

double OperationRegistry::execute(const std::string& opIdentifier, const std::vector<double>& args) const
{
    auto pair = registry.find(opIdentifier);
    if (pair == registry.end()) {
        throw std::runtime_error("Unknown operator: " + opIdentifier);
    }
    return pair->second.procedure(args); // call the registered function
}
};
