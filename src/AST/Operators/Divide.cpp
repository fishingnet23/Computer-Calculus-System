#include "../include/NodeFactory.hpp"

namespace AST
{
    double Divide::evaluate(const std::vector<double>& values) const
    {
        if (values.empty())
            throw std::runtime_error("Division requires at least one argument.");

        double res = values[0];

        for (size_t i = 1; i < values.size(); ++i)
            res /= values[i];

        return res;
    }
    NodePtr Divide::applySymbolicIdentities(OpPtr opNode) const
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
            
            // inverse the child
            auto exponentMinusOne = NodeFactory::makeNumber(-1);
            auto invertedNode = NodeFactory::makeOperator("^", { originalArgs[i], exponentMinusOne });
            auto crushed = toOperator(invertedNode)->simplify();
            args.push_back(crushed);

        }
        

        auto mul = NodeFactory::makeOperator("*",args);
        auto mulOp = toOperator(mul);

        return mul->shared_from_this();
    }
    NodePtr Divide::applySimplification(OpPtr opNode) const
    {
        auto identity = applySymbolicIdentities(opNode);
        auto identityOp = toOperator(identity);
        if(!identityOp)
            return identity;

        auto simplified = identityOp->simplify(); // simplify internally
        return simplified; 
    }

    NodePtr Divide::applyExpansion(OpPtr opNode) const
    {
        auto identity = applySymbolicIdentities(opNode);
        
        auto identityOp = toOperator(identity);
        if(!identityOp)
            return identity;

        auto expanded = identityOp->expand();
        return expanded;
    }

    std::string Divide::toString(const std::vector<NodePtr> & args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            str+=" / "+args[i]->toString();
        }

        return str;
    }
}