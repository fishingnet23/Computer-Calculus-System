#include "../include/NodeFactory.hpp"

namespace AST
{
    double Negate::evaluate(const std::vector<double>& values) const
    {
        if(values.size() != 1)
            throw std::runtime_error("Negate requires exactly one operand!");
        return -values[0];
    }
    NodePtr Negate::applySymbolicIdentities(OpPtr opNode) const
    {
        if(opNode->getArguments().size() != 1)
            throw std::runtime_error("Negate requires exactly one operand!");
        auto child = opNode->getArguments()[0];

        // apply external metadata before collapse
        if(auto* symbol = child->asSymbol())
        {
            symbol->setCoeff(-symbol->getCoeff()*opNode->getCoeff());
            symbol->setDegree(symbol->getDegree()*opNode->getDegree());
        }
        else if(auto* number = child->asNumber())
        {    
            number->setValue(-opNode->getCoeff()*std::pow(number->getValue(),opNode->getDegree()));
        }
        return child;
    }
    NodePtr Negate::applySimplification(OpPtr opNode) const
    {
        auto identity = applySymbolicIdentities(opNode);
        if(!toOperator(identity)) // if the subtraction collapsed into an atomic node
            return identity;
        
        identity = toOperator(identity)->simplify(); // simplify internally
        if(!toOperator(identity)) // if the new operation collapsed into an atomic node
            return identity;
        opNode = std::static_pointer_cast<OperatorNode>(identity);

        return opNode->shared_from_this(); 
    }

    NodePtr Negate::applyExpansion(OpPtr opNode) const
    {
         auto identity = applySymbolicIdentities(opNode);
        if(!toOperator(identity))
            return identity;
        opNode = std::static_pointer_cast<OperatorNode>(identity);

        opNode->expand();

        return opNode->shared_from_this(); 
    }

    std::string Negate::toString(const std::vector<NodePtr> & args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            str+=" - "+args[i]->toString();
        }

        return str;
    }
}