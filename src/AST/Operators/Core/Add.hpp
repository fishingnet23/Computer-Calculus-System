#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{
class Add : public OpDefinition
{
public:
    Add();

    virtual NumberPtr evaluate(const std::vector<NumberPtr>& values, const Environment& env) const override;

    virtual NodePtr complexSimplify(OpPtr opNode) const override;
    virtual NodePtr complexExpansion(OpPtr opNode) const override;
    
    virtual NodePtr complexLocalSimplify(OpPtr opNode) const override;

    virtual std::string toString(const std::vector<NodePtr>& args) const override;
    virtual std::string getName() const override {return "+";}
    virtual Format getFormat() const override {return OpDefinition::Format::INFIX;}
    virtual size_t getPrecedence() const override {return 1;}
    virtual size_t getCanonArgumentCount() const override {return 2;}
    virtual SyntaxRule::SortingFormat getSortingFormat() const override {return SyntaxRule::SortingFormat::DESCEND;}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Add>(*this);};

    virtual OpPtr buildInverseOf(OpPtr opToInvert) const override;
};
}