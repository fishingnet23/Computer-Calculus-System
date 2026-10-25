#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Negate : public SyntaxFluff
{
public:
    Negate():SyntaxFluff({OpProperty::RIGHT_ASSOCIATIVE}){}


    virtual OpPtr clean(const std::vector<NodePtr>& args);

    virtual std::string toString(const std::vector<NodePtr>& args) const override
    {
        if(args.size()!=1)
            throw std::runtime_error("Malformed negation!");
        return "-" + args[0]->toString();
    }
    virtual std::string getName() const override {return "-unary";}
    virtual Format getFormat() const override {return OpDefinition::Format::PREFIX;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 1;}



};
}