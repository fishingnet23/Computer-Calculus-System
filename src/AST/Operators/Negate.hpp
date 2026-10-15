#pragma once
#include "../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Negate : public OpDefinition
{
protected:
    virtual NodePtr applySymbolicIdentities(OpPtr opNode) const override;
public:
    Negate():OpDefinition(){}

    virtual double evaluate(const std::vector<double>& values) const override;
    virtual NodePtr applySimplification(OpPtr opNode) const override;
    virtual NodePtr applyExpansion(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "-";}
    virtual Format getFormat() const override {return OpDefinition::Format::PREFIX;}
    virtual int getPrecedence() const override {return 10;}
    virtual int getCanonArgumentCount() const override {return 1;}

};
}