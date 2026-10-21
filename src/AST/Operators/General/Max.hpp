#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 
#include "NumericLimit.hpp"
namespace AST
{
class Max : public NumericLimit
{
public:
    Max():NumericLimit(){};

    virtual double evaluationFn(double n1,double n2) const override {return std::max(n1,n2);};
    virtual std::string getName() const override {return "max";}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Max>(*this);};


};
}