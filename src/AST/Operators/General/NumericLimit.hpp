#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class NumericLimit : public OpDefinition
{
public:
    NumericLimit():OpDefinition({OpProperty::TRUE_ASSOCIATIVE, OpProperty::COMMUTATIVE}){}

    virtual NumberPtr evaluate(const std::vector<NumberPtr>& values, const Environment& env) const override;

    virtual NumberPtr evaluationFn(NumberPtr n1,NumberPtr n2) const = 0;

    virtual NodePtr complexSimplify(OpPtr opNode) const override;
    virtual NodePtr complexExpansion(OpPtr opNode) const override;
    
    virtual NodePtr complexLocalSimplify(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 2;}


};
}