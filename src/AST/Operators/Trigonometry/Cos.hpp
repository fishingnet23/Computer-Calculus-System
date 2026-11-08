#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Cos : public OpDefinition
{
public:
    Cos();

    virtual NodePtr simplify(OpPtr opNode) const override;
    virtual NodePtr expand(OpPtr opNode) const override;
    
    virtual NodePtr localCollapseStructure(OpPtr opNode) const override;
    virtual InfFloat approximate(const std::vector<InfFloat>& operands) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "cos";}
    virtual Format getFormat() const override {return OpDefinition::Format::EXPLICIT_FUNCTION;}
    virtual size_t getPrecedence() const override {return 4;}
    virtual size_t getCanonArgumentCount() const override {return 1;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Cos>(*this);};


};
}