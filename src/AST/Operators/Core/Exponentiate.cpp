#include "../../include/NodeFactory.hpp"
#include "Exponentiate.hpp"

namespace AST
{
    Exponentiate::Exponentiate():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    NumberPtr Exponentiate::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        if(values.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        
        InfFloat baseFloat, expFloat;
        if (auto base = SoftCasting::toRational(values[0]))
        {
            baseFloat = base->getFraction().convert_to<InfFloat>();
        }
        else if(auto base = SoftCasting::toFloat(values[0]))
        {
            baseFloat = base->getFloat();
        }
        else
            throw std::runtime_error("Invalid scalar base for exponentiation!");

        if (auto exponent = SoftCasting::toRational(values[1]))
        {
            expFloat = exponent->getFraction().convert_to<InfFloat>();
        }
        else if(auto exponent = SoftCasting::toFloat(values[1]))
        {
            expFloat = exponent->getFloat();
        }
        else
            throw std::runtime_error("Invalid scalar exponent for exponentiation!");
        

        InfFloat finalResult = mp::pow(baseFloat, expFloat);

        return NodeFactory::makeNumber(finalResult); 
    }

    NodePtr Exponentiate::complexSimplify(OpPtr opNode) const
    {
        auto expOp = SoftCasting::toOperator(opNode->getArgument(1));

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

        

        auto expOp = SoftCasting::toOperator(exp);

        
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

        if (SoftCasting::toVariable(base) || SoftCasting::toNumber(base))
        {
            return opNode->shared_from_this();
        }
        auto baseOp = SoftCasting::toOperator(base);
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
        InfFloat expVal;
        if(auto expRat = SoftCasting::toRational(exp))
            expVal = expRat->getFraction().convert_to<InfFloat>();
        else if(auto expNum = SoftCasting::toFloat(exp))
            expVal = expNum->getFloat();
        else
            return opNode->shared_from_this();

        if (mp::abs(expVal - mp::round(expVal)) > 1e-9) return opNode->shared_from_this(); 
        if (expVal == 1) return base;

        if (expVal < 0 )
            return opNode->shared_from_this();

        if (baseOp->getName() == "+")
        {
            long long n = static_cast<long long>(expVal);
            NodePtr result = NodeFactory::makeNumber(1);
            NodePtr currentMultiplier = base;

            while (n > 0)
            {
                if (n % 2 == 1)
                {
                    result = NodeFactory::makeOperator("*", {result, currentMultiplier});
                }
                if (n > 1)
                {
                    currentMultiplier = NodeFactory::makeOperator("*", {currentMultiplier, currentMultiplier});
                }
                n /= 2;
            }
            if(n<0)
                result = NodeFactory::makeOperator("^",{result,NodeFactory::makeNumber(-1)});

            // if(auto op = SoftCasting::toOperator(result))
            //     return OperatorNode::expand(op);
            // else
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

        auto baseNum = SoftCasting::toNumber(base);
        auto expNum = SoftCasting::toNumber(exp);

        if (baseNum && expNum)
        {
            return NodeFactory::makeNumber(mp::pow(baseNum->getValue(), expNum->getValue()));
        }
        if (expNum)
        {
            InfFloat expVal = expNum->getValue();
            if (expVal == 0.0)
                return NodeFactory::makeNumber(1);
            if (expVal == 1.0)
                return base; // strip the operator
        }
    
        if (baseNum)
        {
            InfFloat baseVal = baseNum->getValue();
            if (baseVal == 1.0)
                return NodeFactory::makeNumber(1);
            if (baseVal == 0.0)
            {
                if (!expNum || expNum->getValue() > 0)
                    return NodeFactory::makeNumber(0);
            }
        }

        //(a^b)^c = a^(b*c)
        if (auto innerOp = SoftCasting::toOperator(base); innerOp && innerOp->getName() == "^")
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
        if(SoftCasting::toOperator(args[0]))
            str += "(" + args[0]->toString() + ")";
        else
            str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            if(SoftCasting::toOperator(args[i]))
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