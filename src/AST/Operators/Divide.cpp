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
            return child;
        }

        auto& originalArgs = opNode->getArguments(); 
        std::vector<NodePtr> args = { originalArgs[0] };
        args.reserve(originalArgs.size());
        for(int i=1;i<originalArgs.size();i++)
        {
            auto& child = originalArgs[i];
            
            // inverse the child
            NodePtr exponentMinusOne = NodeFactory::makeNumber(-1);
            OpPtr invertedNode = NodeFactory::makeOperatorPtr("^", { originalArgs[i], exponentMinusOne });
            auto crushed = invertedNode->simplify();
            args.push_back(crushed);

        }
        

        auto mul = NodeFactory::makeOperator("*",args);
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