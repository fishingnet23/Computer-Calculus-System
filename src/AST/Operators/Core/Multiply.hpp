#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Multiply : public OpDefinition
{
    bool implicit;
public:
    Multiply():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){this->implicit = false;}
    explicit Multiply(bool implicit):OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){this->implicit = implicit;}

    virtual double evaluate(const std::vector<double>& values, const Environment& env) const override;

    virtual NodePtr applySimplification(OpPtr opNode) const override;
    virtual NodePtr applyExpansion(OpPtr opNode) const override;
    
    virtual NodePtr localSimplification(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "*";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override {return implicit? 3:2;}
    virtual size_t getCanonArgumentCount() const override {return 2;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::static_pointer_cast<OpDefinition>(std::make_shared<Multiply>(*this));};


};
}