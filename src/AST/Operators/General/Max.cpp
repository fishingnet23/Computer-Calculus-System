#include "Max.hpp"
#include "../../include/NodeFactory.hpp"

AST::NumberPtr AST::Max::evaluationFn(NumberPtr n1, NumberPtr n2) const{return NodeFactory::makeNumber(std::max(n1->getValue(),n2->getValue()));};