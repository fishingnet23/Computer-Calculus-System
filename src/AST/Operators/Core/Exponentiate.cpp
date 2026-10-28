#include "../../include/NodeFactory.hpp"
#include "Exponentiate.hpp"

namespace AST
{
    double Exponentiate::evaluate(const std::vector<double>& values, const Environment& env) const
    {
        if(values.size()!= 2)
            throw std::runtime_error("Malformed exponentiation!");
        return std::pow(values[0],values[1]);
    }
    NodePtr Exponentiate::applySimplification(OpPtr opNode) const
    {
        auto expOp = Casting::toOperator(opNode->getArgument(1));

        if(expOp && expOp->getName() == "log")
        {
            auto base = expOp->getArgument(1);
                if(base->identical(opNode->getArgument(0)))
                    return expOp->getArgument(0); // return the inner expression of the log
        }

        return opNode->shared_from_this(); 
    }

    NodePtr Exponentiate::applyExpansion(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        NodePtr base = args[0];
        NodePtr exp = args[1];

        if (Casting::toVariable(base) || Casting::toNumber(base))
        {
            return opNode->shared_from_this();
        }

        auto expNum = Casting::toNumber(exp);
        if (!expNum)
        {
            return opNode->shared_from_this();
        }

        double expVal = expNum->getValue();
        if (expVal <= 0.0) return opNode->shared_from_this(); 
        if (expVal == 1.0) return base;

        auto baseOp = Casting::toOperator(base);
        if (!baseOp) 
        {
            return opNode->shared_from_this();
        }

        if (baseOp->getName() == "*")
        {
            std::vector<NodePtr> distributedArgs;
            for (const auto& child : baseOp->getArguments())
            {
                distributedArgs.push_back(NodeFactory::makeOperator("^", {child, exp}));
            }
            return NodeFactory::makeOperatorPtr("*", distributedArgs)->expand();
        }

        if (baseOp->getName() == "+")
        {
            long long n = static_cast<long long>(expVal);
            
            NodePtr result = NodeFactory::makeNumber(1);
            NodePtr currentMultiplier = base;

            while (n > 0)
            {
                if (n % 2 == 1)
                {
                    result = NodeFactory::makeOperatorPtr("*", {result, currentMultiplier})->expand();
                }
                if (n > 1)
                {
                    currentMultiplier = NodeFactory::makeOperatorPtr("*", {currentMultiplier, currentMultiplier})->expand();
                }
                n /= 2;
            }
            return result;
        }

        return opNode->shared_from_this();
    }


    NodePtr Exponentiate::localSimplification(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        NodePtr base = args[0];
        NodePtr exp = args[1];

        auto baseNum = Casting::toNumber(base);
        auto expNum = Casting::toNumber(exp);

        if (baseNum && expNum)
        {
            return NodeFactory::makeNumber(std::pow(baseNum->getValue(), expNum->getValue()));
        }

        if (expNum)
        {
            double expVal = expNum->getValue();
            if (expVal == 0.0)
                return NodeFactory::makeNumber(1);
            if (expVal == 1.0)
                return base; // strip the operator
        }
    
        if (baseNum)
        {
            double baseVal = baseNum->getValue();
            if (baseVal == 1.0)
                return NodeFactory::makeNumber(1);
            if (baseVal == 0.0)
            {
                if (!expNum || expNum->getValue() > 0)
                    return NodeFactory::makeNumber(0);
            }
        }

        //(a^b)^c = a^(b*c)
        if (auto innerOp = Casting::toOperator(base); innerOp && innerOp->getName() == "^")
        {
            auto& innerArgs = innerOp->getArguments();
            NodePtr innerBase = innerArgs[0];
            NodePtr innerExp = innerArgs[1];

            NodePtr newCombinedExp = NodeFactory::makeOperator("*", {innerExp, exp});
            
            return NodeFactory::makeOperator("^", {innerBase, newCombinedExp});
        }

        return opNode->shared_from_this();
    }
    std::string Exponentiate::toString(const std::vector<NodePtr> &args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        if(Casting::toOperator(args[0]))
            str += "(" + args[0]->toString() + ")";
        else
            str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            if(Casting::toOperator(args[i]))
                str+="^("+args[i]->toString()+")";
            else
                str+= "^" + args[i]->toString();
        }

        return str+"";
    }
}