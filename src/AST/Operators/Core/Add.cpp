#include "../../include/NodeFactory.hpp"
#include "Add.hpp"

namespace AST
{
    Add::Add():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){}

    NumberPtr Add::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        bool allRational = true;
        for (const auto& arg : values) {
            if (!SoftCasting::toRational(arg)) {
                allRational = false;
                break;
            }
        }
        if (allRational) {
            InfRational r(0, 1);
            for (const auto& arg : values) {
                r += SoftCasting::toRational(arg)->getFraction();
            }
            return NodeFactory::makeNumber(r);
        }
        
        InfFloat f(0);

        for (const auto& arg : values) {
            if (auto rat = SoftCasting::toRational(arg)) {
                f += rat->getFraction().convert_to<InfFloat>();
            } 
            else if (auto f2 = SoftCasting::toFloat(arg)) {
                f += f2->getFloat();
            } 
            else {
                f += InfFloat(arg->getValue());
            }
        }

        return NodeFactory::makeNumber(f);
    }

    NodePtr Add::complexSimplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        SymbolDataAccumulator coeffMap;
        coeffMap.setConstant(0);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toNumber(node))
            {
                coeffMap.setConstant(coeffMap.getConstant()+number->getValue());
                continue;
            }
            InfFloat coeff(1);
            if(const auto& symbol = SoftCasting::toSymbol(node))
            {
                SymbolPtr final = symbol;
                if(const auto& innerOp = SoftCasting::toOperator(node))
                {
                    if(innerOp->getName() == "*")
                    {
                        auto& innerArgs = innerOp->getArguments();
                        // assume multiplication is not malformed and that it is already canonicalized
                        if(const auto& coeffNode = SoftCasting::toNumber(innerArgs[0]))
                        {
                            coeff = coeffNode->getValue();
                            final = SoftCasting::toSymbol(innerOp->remakeWithoutArgument(0));
                        }
                    }
                }
                coeffMap.insertOrAccumulate(final->getPostFix(),final,[coeff](InfFloat& accumCoeff,InfFloat& accumDeg)
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
        InfFloat constant(0);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toNumber(node))
            {
                if(number->getValue() == 0)
                    continue;
                constant += number->getValue();
                continue;
            }

            newArgs.push_back(node);
        }

        if (newArgs.empty())
            return NodeFactory::makeNumber(constant);
        if(constant != 0)
            newArgs.push_back(NodeFactory::makeNumber(constant));
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

    OpPtr Add::buildInverseOf(OpPtr opToInvert) const
    {
        if(opToInvert->getDefinition()->getName() != getName())
            return nullptr; // not the same operator

        auto& args = opToInvert->getArguments();
        std::vector<NodePtr> newArgs;
        newArgs.reserve(args.size());

        // The addition operator is invertible by negating all of its arguments, 
        // so we can build the inverse by negating all of the arguments of the inputted operator

        // Adding the new operator and this operator will always simplify to 0, the addative identiy.
        for(const auto& arg: args)
        {
            auto negated = NodeFactory::makeOperator("*",{NodeFactory::makeNumber(-1),arg});
            newArgs.push_back(negated);
        }

        return NodeFactory::makeOperator(getName(),newArgs);
    }
}
