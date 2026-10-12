#include "../include/NodeFactory.hpp"

namespace AST
{
    double Subtract::evaluate(const std::vector<double>& values) const
    {
        if(values.size() < 1)
            return 0.0;
        double res = values[0];
        for(const auto& value:values)
        {
            res -= value;
        }
        return res;
    }
    NodePtr Subtract::applySymbolicIdentities(OpPtr opNode) const
    {
        if(opNode->getArguments().size() == 0)
            return NodeFactory::makeNumber(0);
        if(opNode->getArguments().size() == 1)
        {
            auto child = opNode->getArguments()[0];

            // apply external metadata before collapse
            if(auto* symbol = child->asSymbol())
            {
                symbol->setCoeff(symbol->getCoeff()*opNode->getCoeff());
                symbol->setDegree(symbol->getDegree()*opNode->getDegree());
            }
            else if(auto* number = child->asNumber())
            {    
                number->setValue(opNode->getCoeff()*std::pow(number->getValue(),opNode->getDegree()));
            }
            return child;
        }

        auto& originalArgs = opNode->getArguments(); 
        std::vector<NodePtr> args = { originalArgs[0] };
        args.reserve(originalArgs.size());
        for(int i=1;i<originalArgs.size();i++)
        {
            auto& child = originalArgs[i];
            
            // negate the child
            if(auto* symbol = child->asSymbol())
            {
                symbol->setCoeff(symbol->getCoeff()*-1);
            }
            else if(auto* number = child->asNumber())
            {    
                number->setValue(-number->getValue());
            }

            args.push_back(child);
        }
        

        auto addition = NodeFactory::makeOperator("+",args);
        return addition->shared_from_this();
    }
    NodePtr Subtract::applySimplification(OpPtr opNode) const
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

    NodePtr Subtract::applyExpansion(OpPtr opNode) const
    {
         auto identity = applySymbolicIdentities(opNode);
        if(!toOperator(identity))
            return identity;
        opNode = std::static_pointer_cast<OperatorNode>(identity);

        opNode->expand();

        return opNode->shared_from_this(); 
    }

    std::string Subtract::toString(const std::vector<NodePtr> & args) const
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