#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Derivative : public OpDefinition
{
public:
    Derivative():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    virtual double evaluate(const std::vector<double>& values) const override;

    virtual NodePtr applySimplification(OpPtr opNode) const override;
    virtual NodePtr applyExpansion(OpPtr opNode) const override;
    
    virtual NodePtr localSimplification(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "derivative";}
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 2;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Derivative>(*this);};


};
}