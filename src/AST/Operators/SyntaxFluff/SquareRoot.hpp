#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class SquareRoot : public SyntaxFluff
{
public:
    SquareRoot():SyntaxFluff({OpProperty::RIGHT_ASSOCIATIVE}){}


    virtual OpPtr clean(const std::vector<NodePtr>& args);

    virtual std::string toString(const std::vector<NodePtr>& args) const override
    {
        if(args.size()!=1)
            throw std::runtime_error("Malformed sqrt!");
        return "sqrt("+ args[0]->toString() + ")";
    }
    virtual std::string getName() const override {return "sqrt";}
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 1;}



};
}