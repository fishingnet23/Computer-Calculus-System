#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Subtract : public SyntaxFluff
{
public:
    Subtract():SyntaxFluff({OpProperty::LEFT_ASSOCIATIVE}){}


    virtual OpPtr clean(std::vector<NodePtr>& args);

    virtual std::string toString(const std::vector<NodePtr>& args) const override
    {
        if(args.size()!=2)
            throw std::runtime_error("Malformed subtraction!");
        return args[0]->toString() + "-" + args[1]->toString();
    }
    virtual std::string getName() const override {return "-";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override {return 1;}
    virtual size_t getCanonArgumentCount() const override {return 2;}



};
}