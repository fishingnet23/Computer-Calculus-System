#pragma once
#include "../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Divide : public SyntaxFluff
{
public:
    Divide():SyntaxFluff({OpProperty::RIGHT_ASSOCIATIVE}){}


    virtual OpPtr clean(std::vector<NodePtr>& args);

    virtual std::string toString(const std::vector<NodePtr>& args) const override
    {
        if(args.size()!=2)
            throw std::runtime_error("Malformed division!");
        return args[0]->toString() + "/" + args[1]->toString();
    }
    virtual std::string getName() const override {return "/";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override {return 2;}
    virtual size_t getCanonArgumentCount() const override {return 2;}



};
}