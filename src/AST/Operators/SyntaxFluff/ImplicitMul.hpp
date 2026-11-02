#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class ImplicitMul : public SyntaxFluff
{
public:
    ImplicitMul():SyntaxFluff({OpProperty::TRUE_ASSOCIATIVE, OpProperty::COMMUTATIVE}){}

    virtual OpPtr clean(const std::vector<NodePtr>& args);

    virtual std::string toString(const std::vector<NodePtr>& args) const override
    {
        return "";
    }
    virtual std::string getName() const override {return "[implicit*]";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override {return 3;}
    virtual size_t getCanonArgumentCount() const override {return 2;}



};
}