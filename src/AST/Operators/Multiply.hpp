#pragma once
#include "../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class 
Multiply : public OpDefinition
{
protected:
    bool implicit;
    virtual NodePtr applySymbolicIdentities(OpPtr opNode) const override;
public:
    Multiply():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){this->implicit = false;}
    Multiply(bool implicit):OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){this->implicit = implicit;}

    virtual double evaluate(const std::vector<double>& values) const override;
    virtual NodePtr applySimplification(OpPtr opNode) const override;
    virtual NodePtr applyExpansion(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "*";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual int getPrecedence() const override {return implicit? 3:2;}
};
}