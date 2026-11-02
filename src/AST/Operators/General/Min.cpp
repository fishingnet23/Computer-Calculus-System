#include "Min.hpp"
#include "../../include/NodeFactory.hpp"

AST::NumberPtr AST::Min::evaluationFn(NumberPtr n1, NumberPtr n2) const{return NodeFactory::makeNumber(std::min(n1->getValue(),n2->getValue()));}