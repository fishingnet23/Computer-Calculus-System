#include "Operators.hpp"
namespace AST
{
OperationRegistry::OperationRegistry()
{
    // binary operators
    auto add = B_Operation(1,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
        return args[0] + args[1];
        });
    auto sub = B_Operation(1,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
        return args[0] - args[1];
        });
    auto mul = B_Operation(2,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        return args[0] * args[1];
        });
    auto pow = B_Operation(3,[](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'^' requires 2 arguments");
        return std::pow(args[0], args[1]);
        });

    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::COMMUTATIVE));
    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));
    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));

    sub.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));

    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::COMMUTATIVE));
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::DISTRIBUTIVE, {&add, &sub}));

    pow.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));

    registry["+"] = std::make_unique<B_Operation>(add);
    registry["-"] = std::make_unique<B_Operation>(sub);
    registry["*"] = std::make_unique<B_Operation>(mul);
    registry["^"] = std::make_unique<B_Operation>(pow);

    // unary operators

    // unary minus, negates the value of its single argument unlike subtraction
    registry["unary-"] = std::make_unique<Operation>(Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'-' requires 1 argument"); 
        return -args[0];
        },Operation::Format::PREFIX));

    registry["sin"] = std::make_unique<Operation>(Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        },Operation::Format::PREFIX));
    registry["cos"] = std::make_unique<Operation>(Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument");
        return std::cos(args[0]);
        }, Operation::Format::PREFIX));
    registry["ln"] = std::make_unique<Operation>(Operation(10,[](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'ln' requires 1 argument");
        return std::log(args[0]);
        }, Operation::Format::PREFIX));

}

double OperationRegistry::execute(const std::string& opIdentifier, const std::vector<double>& args) const
{
    auto pair = registry.find(opIdentifier);
    if (pair == registry.end()) {
        throw std::runtime_error("Unknown operator: " + opIdentifier);
    }
    return pair->second->procedure(args);
}
const Operation * OperationRegistry::getOperation(const std::string & op) const
{
    auto pair = registry.find(op);
    return pair != registry.end() ? pair->second.get() : nullptr;
}
bool B_Operation::hasProperty(B_Operation::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return true;
    }
    return false;   
}
};
