#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 
#include "NumericLimit.hpp"
namespace AST
{
class Min : public NumericLimit
{
public:
    Min():NumericLimit(){};

    virtual double evaluationFn(double n1,double n2) const override {return std::min(n1,n2);};
    virtual std::string getName() const override {return "min";}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Min>(*this);};


};
}