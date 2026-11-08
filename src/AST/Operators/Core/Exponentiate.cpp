#include "../../include/NodeFactory.hpp"
#include "Exponentiate.hpp"
#include "../../include/Utilities.hpp"


namespace AST
{



    Exponentiate::Exponentiate():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}


    InfFloat Exponentiate::approximate(const std::vector<InfFloat> &operands) const
    {
        if(operands.size()!=2)
            throw std::runtime_error("Malformed exponentiation!");

        return mp::pow(operands[0],operands[1]);
    }

    NodePtr Exponentiate::simplify(OpPtr opNode) const
    {
        auto expOp = SoftCasting::toOperator(opNode->getArgument(1));

        if(expOp && expOp->getName() == "log")
        {
            auto base = expOp->getArgument(1);
                if(base == opNode->getArgument(0))
                    return expOp->getArgument(0); // return the inner expression of the log
        }

        return opNode->shared_from_this(); 
    }

    NodePtr Exponentiate::expand(OpPtr opNode) const
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
        if(auto numericBase = SoftCasting::toRational(base))
        {
            if(auto numericExponent = SoftCasting::toRational(exp))
            {
                auto baseFrac = numericBase->getFraction();
                auto expFrac = numericExponent->getFraction();
                // there's a hidden complex number in here
                if (baseFrac < 0 && (denominator(expFrac) % 2 == 0))
                {
                    auto iPower = numerator(expFrac); 
                    
                    auto posBase = NodeFactory::makeNumber(-baseFrac);
                    auto realPart = NodeFactory::makeOperator("^", {posBase, exp});
                    
                    auto iSymbol = NodeFactory::makeVariable("i");
                    NodePtr imagPart;
                    if (iPower == 1) {
                        imagPart = iSymbol;
                    } else {
                        imagPart = NodeFactory::makeOperator("^", {iSymbol, NodeFactory::makeNumber(iPower)});
                    }
                    
                    return NodeFactory::makeOperator("*", {realPart, imagPart});
                }
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
        InfRational expVal;
        if(auto expRat = SoftCasting::toRational(exp))
            expVal = expRat->getFraction();
        else
            return opNode->shared_from_this();

        if (denominator(expVal) != 1) return opNode->shared_from_this(); 
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

            if(auto op = SoftCasting::toOperator(result))
                return OperatorNode::expand(op);
            else
                return result;
        }
        
        return opNode->shared_from_this();
    }

    NodePtr Exponentiate::localCollapseStructure(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        NodePtr base = args[0];
        NodePtr exp = args[1];

        auto baseNum = SoftCasting::toRational(base);
        auto expNum = SoftCasting::toRational(exp);
        auto baseVar = SoftCasting::toVariable(base);

        // check if the base is the imaginary identity i, if it is, cycle the degree
        if (baseVar && baseVar->getName() == "i" && expNum)
        {
            auto expFrac = expNum->getFraction();
            if (denominator(expFrac) == 1)
            {
                long long power = numerator(expFrac).convert_to<long long>();
                
                long long rem = power % 4;
                if (rem < 0) rem += 4;

                if (rem == 0) return NodeFactory::makeNumber(1);
                if (rem == 1) return base; // returns i
                if (rem == 2) return NodeFactory::makeNumber(-1);
                if (rem == 3) {
                    // Returns -i as (-1 * i)
                    return NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), base});
                }
            }
        }
        if (baseNum)
        {
            InfFloat baseVal = baseNum->getValue();
            if (baseVal == 1.0)
                return NodeFactory::makeNumber(1);
            if (baseVal == 0.0)
            {
                if (expNum && expNum->getValue() == 0)
                    throw std::runtime_error("Indeterminate form: 0^0");
                if (!expNum || expNum->getValue() > 0)
                    return NodeFactory::makeNumber(0);
            }

        }
        if (expNum)
        {
            auto expFrac = expNum->getFraction();
            
            if (expFrac == 0)
                return NodeFactory::makeNumber(1);
            if (expFrac == 1)
                return base; // strip the operator
        
            if (baseNum && denominator(expFrac) == 1)
            {  
                auto baseFrac = baseNum->getFraction();
                auto p = mp::pow(numerator(baseFrac),mp::abs(numerator(expFrac)).convert_to<unsigned long>());
                auto q = mp::pow(denominator(baseFrac),mp::abs(numerator(expFrac)).convert_to<unsigned long>());
                InfRational res;
                if(numerator(expFrac) >= 0 || p==0)
                    res = InfRational(p,q);
                else
                    res = InfRational(q,p);

                return NodeFactory::makeNumber(res);
            }
            // integer radical, try to simplify
            else if(baseNum && baseNum->getValue() >= 0 && baseNum->getValue() == mp::floor(baseNum->getValue()) && numerator(expFrac) == 1)
            {
                auto radicand = numerator(baseNum->getFraction());
                unsigned long long rootSize = denominator(expFrac).convert_to<unsigned long long>();
                InfInt outCoeff;
                InfInt outRadicand;
                Utilities::simplifyRoot(rootSize,radicand,outCoeff,outRadicand);

                if(outRadicand == 1) {
                    return NodeFactory::makeNumber(outCoeff);
                }
                auto remainderRootNode = NodeFactory::makeOperator("^", {NodeFactory::makeNumber(outRadicand), exp});

                if(outCoeff == 1) {
                    return remainderRootNode;
                }
                return OperatorNode::simplify(NodeFactory::makeOperator("*", {NodeFactory::makeNumber(outCoeff), remainderRootNode}));

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
            if(SoftCasting::toOperator(args[i]) || (SoftCasting::toRational(args[i]) && denominator(SoftCasting::toRational(args[i])->getFraction())!=1))
                str+="^("+args[i]->toString()+")";
            else
                str+= "^" + args[i]->toString();
        }

        return str+"";
    }
}