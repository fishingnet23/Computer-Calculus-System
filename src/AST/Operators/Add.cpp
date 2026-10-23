#include "../include/NodeFactory.hpp"
#include "Add.hpp"

namespace AST
{
    double Add::evaluate(const std::vector<double>& values) const
    {
        double res = 0;
        for(const auto& value:values)
        {
            res += value;
        }
        return res;
    }
    NodePtr Add::applySimplification(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        SymbolDataAccumulator coeffMap;
        coeffMap.setConstant(0);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = toNumber(node))
            {
                coeffMap.setConstant(coeffMap.getConstant()+number->getValue());
                continue;
            }
            double coeff = 1;
            if(const auto& symbol = toSymbol(node))
            {
                SymbolPtr final = symbol;
                if(const auto& innerOp = toOperator(node))
                {
                    if(innerOp->getName() == "*")
                    {
                        auto& innerArgs = innerOp->getArguments();
                        // assume multiplication is not malformed and that it is already canonicalized
                        if(const auto& coeffNode = toNumber(innerArgs[0]))
                        {
                            coeff = coeffNode->getValue();
                            final = toSymbol(innerOp->remakeWithoutArgument(0));
                        }
                    }
                }
                coeffMap.insertOrAccumulate(final->getPostFix(),final,[coeff](double& accumCoeff,double& accumDeg)
                {
                    accumCoeff+=coeff;
                },coeff,1);
            }
            
            // unknowns are ignored, though they shouldn't exist
        }

        std::vector<NodePtr> reconstructed;
        reconstructed.reserve(coeffMap.get().size() + 1);
        
        if(coeffMap.getConstant()!= 0)
            reconstructed.push_back(NodeFactory::makeNumber(coeffMap.getConstant()));
        
        for(const auto& [name,data] :coeffMap.get())
        {
            if(data.coeff == 0)
                continue;

            NodePtr nodeToAdd;
            if(data.coeff == 1)
                nodeToAdd = data.originalNode;
            else
            {
                nodeToAdd = NodeFactory::makeOperator("*",{NodeFactory::makeNumber(data.coeff),data.originalNode});
            }

            reconstructed.push_back(nodeToAdd);
        }

        opNode->setArguments(reconstructed); // wash away the old structure at this node

        return localSimplification(opNode); // final collapse handling in case of a malformed addition
    }
    NodePtr Add::applyExpansion(OpPtr opNode) const
    {
        return opNode->shared_from_this();
    }

    NodePtr Add::localSimplification(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());

        for(const auto& node: oldArgs)
        {
            if(const auto& number = toNumber(node))
            {
                if(number->getValue() == 0)
                    continue;
            }

            newArgs.push_back(node);
        }

        if (newArgs.empty())
            return NodeFactory::makeNumber(0);
        if(newArgs.size() == 1)
            return newArgs[0]; // collapse to child

        opNode->setArguments(newArgs);
        return opNode;
    }
    std::string Add::toString(const std::vector<NodePtr> &args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            str+=" + " + args[i]->toString();
        }

        return str;
    }
}
