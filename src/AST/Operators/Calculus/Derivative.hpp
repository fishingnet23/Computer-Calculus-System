#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Derivative : public OpDefinition
{
public:
    Derivative();

    virtual NodePtr simplify(OpPtr opNode) const override;
    virtual NodePtr expand(OpPtr opNode) const override;
    
    virtual NodePtr localCollapseStructure(OpPtr opNode) const override;
    virtual InfFloat approximate(const std::vector<InfFloat>& operands) const override {throw std::runtime_error(getName()+": Higher order operation cannot be approximated!");};

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "derivative";}
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 2;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Derivative>(*this);};

    virtual OpPtr buildInverseOf(OpPtr opToInvert) const override;
};
}