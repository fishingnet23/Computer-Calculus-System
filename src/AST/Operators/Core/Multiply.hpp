#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Multiply : public OpDefinition
{
public:
    Multiply();

    virtual NumberPtr evaluate(const std::vector<NumberPtr>& values, const Environment& env) const override;

    virtual NodePtr complexSimplify(OpPtr opNode) const override;
    virtual NodePtr complexExpansion(OpPtr opNode) const override;
    
    virtual NodePtr complexLocalSimplify(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "*";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override { return 2;}
    virtual size_t getCanonArgumentCount() const override {return 2;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::static_pointer_cast<OpDefinition>(std::make_shared<Multiply>(*this));};

    virtual OpPtr buildInverseOf(OpPtr opToInvert) const override;
};
}