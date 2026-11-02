#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 
#include "NumericLimit.hpp"
namespace AST
{
class Max : public NumericLimit
{
public:
    Max():NumericLimit(){};

    virtual NumberPtr evaluationFn(NumberPtr n1,NumberPtr n2) const override;
    virtual std::string getName() const override {return "max";}
    virtual std::shared_ptr<OpDefinition> clone() const override{return std::make_shared<Max>(*this);};


};
}