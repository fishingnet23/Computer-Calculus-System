#pragma once
#include "Operators.hpp"
#include "TreeNodes.h"
namespace AST
{
OperationRegistry::OperationRegistry()
{
    // binary operators
    B_Operation add;
    add.precedence = 1;
    add.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
            return args[0] + args[1];
        };
    
    B_Operation sub;
    sub.precedence = 1;
    sub.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
            return args[0] - args[1];
        };
    B_Operation mul; 
    mul.precedence = 2;
    mul.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
            return args[0] * args[1];
        };
    B_Operation pow;
    pow.precedence = 3;
    pow.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'^' requires 2 arguments");
            return std::pow(args[0], args[1]);
        };

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
    Operation unaryMinus;
    unaryMinus.precedence = 10;
    unaryMinus.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'-' requires 1 argument"); 
        return -args[0];
        };
    unaryMinus.format = Operation::Format::PREFIX;

    Operation sin;
    sin.precedence = 10;
    sin.procedure = [](const std::vector<double>& args) {
       if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        };
    sin.format = Operation::Format::PREFIX;
    
    Operation cos;
    cos.precedence = 10;
    cos.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument"); 
        return std::cos(args[0]);
        };
    cos.format = Operation::Format::PREFIX;

    

    Operation ln;
    ln.precedence = 10;
    ln.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'ln' requires 1 argument"); 
        return std::log(args[0]);
        };
    ln.format = Operation::Format::PREFIX;


    registry["unary-"] = std::make_unique<Operation>(unaryMinus);
    registry["sin"] = std::make_unique<Operation>(sin);
    registry["cos"] = std::make_unique<Operation>(cos);
    registry["ln"] = std::make_unique<Operation>(ln);
}

double OperationRegistry::execute(const std::string& opIdentifier, const std::vector<double>& args) const
{
    auto pair = registry.find(opIdentifier);
    if (pair == registry.end()) {
        throw std::runtime_error("Unknown operator: " + opIdentifier);
    }
    return pair->second->procedure(args);
}
NodePtr OperationRegistry::simplifyWithIdentities(NodePtr op) const
{

    const Operation* operation = getOperation(op->getName());
    if(!operation)
        return op;
    return operation->identityfn(op);
}

const Operation *OperationRegistry::getOperation(const std::string &op) const
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
const B_Operation::PropertyFlag *B_Operation::getProperty(B_Operation::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return &propFlag;
    }
    return nullptr;
}
};
