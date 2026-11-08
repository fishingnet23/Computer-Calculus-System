#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 
#include "NumericLimit.hpp"
namespace AST
{
class Min : public NumericLimit
{
public:
    Min():NumericLimit(){};

    virtual NumberPtr evaluationFn(NumberPtr n1,NumberPtr n2) const override;
    virtual InfFloat approximationFn(const InfFloat& n1,const InfFloat& n2) const override;
    virtual std::string getName() const override {return "min";}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Min>(*this);};


};
}