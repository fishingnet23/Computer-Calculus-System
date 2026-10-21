#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class NumericLimit : public OpDefinition
{
public:
    NumericLimit():OpDefinition({OpProperty::TRUE_ASSOCIATIVE, OpProperty::COMMUTATIVE}){}

    virtual double evaluate(const std::vector<double>& values) const override;

    virtual double evaluationFn(double n1,double n2) const = 0;

    virtual NodePtr applySimplification(OpPtr opNode) const override;
    virtual NodePtr applyExpansion(OpPtr opNode) const override;
    
    virtual NodePtr localSimplification(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 2;}


};
}