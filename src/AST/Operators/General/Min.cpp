#include "Min.hpp"
#include "../../include/NodeFactory.hpp"

AST::NumberPtr AST::Min::evaluationFn(NumberPtr n1, NumberPtr n2) const
{
    if(n1->getValue() < n2->getValue())
        return n1;
    else
        return n2;
}

AST::InfFloat AST::Min::approximationFn(const AST::InfFloat& n1, const AST::InfFloat& n2) const
{
    return std::min(n1,n2);
}
