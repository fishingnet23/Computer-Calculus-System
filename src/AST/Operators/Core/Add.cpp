#include "../../include/NodeFactory.hpp"
#include "Add.hpp"

namespace AST
{
    Add::Add():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){}

    NumberPtr Add::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        RationalNode r(0,1);

        for(const auto& arg:values)
        {
            if(auto rat = Casting::toRational(arg))
                r = r + (*rat);
            else
                throw std::runtime_error("Irrational addition unsupported!");
        }

        return NodeFactory::makeNumber(r.getNumerator(),r.getDenominator());
    }
    NodePtr Add::complexSimplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        SymbolDataAccumulator coeffMap;
        coeffMap.setConstant(0);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = Casting::toNumber(node))
            {
                coeffMap.setConstant(coeffMap.getConstant()+number->getValue());
                continue;
            }
            double coeff = 1;
            if(const auto& symbol = Casting::toSymbol(node))
            {
                SymbolPtr final = symbol;
                if(const auto& innerOp = Casting::toOperator(node))
                {
                    if(innerOp->getName() == "*")
                    {
                        auto& innerArgs = innerOp->getArguments();
                        // assume multiplication is not malformed and that it is already canonicalized
                        if(const auto& coeffNode = Casting::toNumber(innerArgs[0]))
                        {
                            coeff = coeffNode->getValue();
                            final = Casting::toSymbol(innerOp->remakeWithoutArgument(0));
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

        opNode = NodeFactory::getWithArguments(opNode,reconstructed); // wash away the old structure at this node

        return complexLocalSimplify(opNode); // final collapse handling in case of a malformed addition
    }
    NodePtr Add::complexExpansion(OpPtr opNode) const
    {
        return opNode->shared_from_this();
    }

    NodePtr Add::complexLocalSimplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());

        for(const auto& node: oldArgs)
        {
            if(const auto& number = Casting::toNumber(node))
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

        opNode = NodeFactory::getWithArguments(opNode,newArgs);
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
