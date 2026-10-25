#include "../../include/NodeFactory.hpp"
#include "Multiply.hpp"

namespace AST
{
    double Multiply::evaluate(const std::vector<double>& values, const Environment& env) const
    {
        double res = 1;
        for(const auto& value:values)
        {
            res *= value;
        }
        return res;
    }
    NodePtr Multiply::applySimplification(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        SymbolDataAccumulator coeffMap;
        coeffMap.setConstant(1);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = Casting::toNumber(node))
            {
                coeffMap.setConstant(coeffMap.getConstant()*number->getValue());
                continue;
            }
            double degree = 1;
            if(const auto& symbol = Casting::toSymbol(node))
            {
                SymbolPtr final = symbol;
                if(const auto& innerOp = Casting::toOperator(node))
                {
                    auto& innerArgs = innerOp->getArguments();
                    if(innerOp->getName() == "^")
                    {    
                        // assume exponentiation is not malformed and that it is already canonicalized
                        if(const auto& expNode = Casting::toNumber(innerArgs[1])) // check if the exponent is numeric 
                        {
                            // strip it
                            degree = expNode->getValue();
                            final = Casting::toSymbol(innerArgs[0]);
                        }
                    }
                    
                }
                coeffMap.insertOrAccumulate(final->getPostFix(),final,[degree](double& accumCoeff,double& accumDeg)
                {
                    accumDeg += degree; // (a^b)*(a^c) = a^(b+c) 
                },1,degree);
            }
            
            // unknowns are ignored, though they shouldn't exist
        }

        std::vector<NodePtr> reconstructed;
        reconstructed.reserve(coeffMap.get().size() + 1);
        
        if(coeffMap.getConstant()!= 1)
            reconstructed.push_back(NodeFactory::makeNumber(coeffMap.getConstant()));
        
        for(const auto& [name,data] :coeffMap.get())
        {
            NodePtr nodeToMultiply;
            if(data.degree == 0)
                nodeToMultiply = NodeFactory::makeNumber(1); // placeholder, removed later
            else if(data.degree == 1)
                nodeToMultiply = data.originalNode;
            else
            {
                nodeToMultiply = NodeFactory::makeOperator("^",{data.originalNode,NodeFactory::makeNumber(data.degree)});
            }

            reconstructed.push_back(nodeToMultiply);
        }

        opNode->setArguments(reconstructed); // wash away the old structure at this node

        return localSimplification(opNode); // final collapse handling in case of a malformed multiplication / final simplification
    }

    NodePtr Multiply::applyExpansion(OpPtr opNode) const
    {

        // a(b*c) = a*b+a*c
        std::vector<std::vector<NodePtr>> currentCombinations = {{}};

        for(const auto& arg : opNode->getArguments())
        {
            std::vector<std::vector<NodePtr>> nextCombinations;
            
            if(const auto& addOp = Casting::toOperator(arg); addOp && addOp->getName() == "+") // distribute over addition
            {
                for(const auto& existingTerms : currentCombinations)
                {
                    for(const auto& addArg : addOp->getArguments())
                    {
                        auto newCombination = existingTerms;
                        newCombination.push_back(addArg->clone());
                        nextCombinations.push_back(newCombination);
                    }
                }
            }
            else
            {
                for(auto existingTerms : currentCombinations)
                {
                    existingTerms.push_back(arg->clone());
                    nextCombinations.push_back(existingTerms);
                }
            }
            currentCombinations = std::move(nextCombinations);
        }

        std::vector<NodePtr> sumArguments;
        for(const auto& termGroup : currentCombinations)
        {
            if(termGroup.size() == 1)
            {
                sumArguments.push_back(termGroup[0]);
            }
            else
            {
                sumArguments.push_back(NodeFactory::makeOperator("*", termGroup));
            }
        }

        if(sumArguments.size() == 1)
        {
            return sumArguments[0];
        }

        return NodeFactory::makeOperatorPtr("+", sumArguments);
    }


    NodePtr Multiply::localSimplification(OpPtr opNode) const
    {

        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());

        for(const auto& node: oldArgs)
        {
            if(const auto& number = Casting::toNumber(node))
            {
                if(number->getValue() == 0)
                    return NodeFactory::makeNumber(0);
                if(number->getValue() == 1)
                    continue;
            }

            newArgs.push_back(node);
        }

        if (newArgs.empty())
            return NodeFactory::makeNumber(1);
        if(newArgs.size() == 1)
            return newArgs[0]; // collapse to child

        opNode->setArguments(newArgs);
        return opNode;
    }
    std::string Multiply::toString(const std::vector<NodePtr> &args) const
    {
        std::string str = "(";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            auto prevNumber = Casting::toNumber(args[i-1]);
            auto currNumber = Casting::toNumber(args[i]);
            auto prevOp = Casting::toOperator(args[i-1]);
            if(prevNumber && currNumber || prevOp)
                str+=" * " + args[i]->toString();
            else
                str+= " "+args[i]->toString();
        }

        return str+")";
    }
}