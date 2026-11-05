#include "../../include/NodeFactory.hpp"
#include "Exponentiate.hpp"

namespace AST
{
    Exponentiate::Exponentiate():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    NumberPtr Exponentiate::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        if(values.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        auto base = Casting::toRational(values[0]);
        auto argument = Casting::toRational(values[1]);
        
        if (!argument || !base)
            throw std::runtime_error("Irrational exponentiation not implemented!");
        
        // Cast the numerator to double to force floating-point division!
        double exponent_value = static_cast<double>(argument->getNumerator()) / static_cast<double>(argument->getDenominator());
        
        return NodeFactory::makeNumber(
            std::pow(static_cast<double>(base->getNumerator()), exponent_value), 
            std::pow(static_cast<double>(base->getDenominator()), exponent_value)
        ); 
    }

    NodePtr Exponentiate::complexSimplify(OpPtr opNode) const
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

    NodePtr Exponentiate::complexExpansion(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        NodePtr base = args[0];
        NodePtr exp = args[1];

        

        auto expOp = Casting::toOperator(exp);

        
        if(expOp)
        {
            // x^(a+b) -> x^a*x^b
            if(expOp->getName() == "+")
            {
                const auto& expArgs = expOp->getArguments();
                std::vector<NodePtr> termsToMultiply;
                termsToMultiply.reserve(expArgs.size());

                for(const auto& arg:expArgs)
                {
                    termsToMultiply.push_back(NodeFactory::makeOperator("^",{base,arg}));
                }

                return NodeFactory::makeOperator("*",termsToMultiply);
            }
        }

        if (Casting::toVariable(base) || Casting::toNumber(base))
        {
            return opNode->shared_from_this();
        }
        auto baseOp = Casting::toOperator(base);
        if (!baseOp) 
        {
            return opNode->shared_from_this();
        }

        if (baseOp->getName() == "*")
        {
            std::vector<NodePtr> distributedArgs;
            distributedArgs.reserve(baseOp->getArguments().size());
            for (const auto& child : baseOp->getArguments())
            {
                distributedArgs.push_back(NodeFactory::makeOperator("^", {child, exp}));
            }
            return OperatorNode::expand(NodeFactory::makeOperator("*", distributedArgs));
        }
        
        auto expNum = Casting::toNumber(exp);
        if (!expNum)
        {
            return opNode->shared_from_this();
        }

        double expVal = expNum->getValue();
        if (std::abs(expVal - std::round(expVal)) > 1e-9) return opNode->shared_from_this(); 
        if (expVal == 1.0) return base;

        

        if (baseOp->getName() == "+")
        {
            long long n = std::abs(static_cast<long long>(expVal));
            NodePtr result = NodeFactory::makeNumber(1);
            NodePtr currentMultiplier = base;

            while (n > 0)
            {
                if (n % 2 == 1)
                {
                    result = OperatorNode::expand(NodeFactory::makeOperator("*", {result, currentMultiplier}));
                }
                if (n > 1)
                {
                    currentMultiplier = OperatorNode::expand(NodeFactory::makeOperator("*", {currentMultiplier, currentMultiplier}));
                }
                n /= 2;
            }
            if(expVal < 0)
                result = NodeFactory::makeOperator("^",{result,NodeFactory::makeNumber(-1)});
            return result;
        }

        return opNode->shared_from_this();
    }


    NodePtr Exponentiate::complexLocalSimplify(OpPtr opNode) const
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

    OpPtr Exponentiate::buildInverseOf(OpPtr opToInvert) const
    {
        if(opToInvert->getDefinition()->getName() != getName())
            return nullptr; // not the same operator

        auto& args = opToInvert->getArguments();
        if(args.size() != 2)
            return nullptr; // malformed exponentiation

        auto base = args[0];
        auto exp = args[1];

        return NodeFactory::makeOperator("log",{exp,base}); // the inverse of a^b is log_a(b)
        // in my notation, log(b,a) = log_a(b)
    }
}