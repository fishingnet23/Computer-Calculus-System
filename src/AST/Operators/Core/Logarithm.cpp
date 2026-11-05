#include "../../include/NodeFactory.hpp"
#include "Logarithm.hpp"

namespace AST
{
    Logarithm::Logarithm():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    NumberPtr Logarithm::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        if (values.size() != 2)
            throw std::runtime_error("Malformed logarithm!");
        
        auto argument = Casting::toRational(values[0]);
        auto base = Casting::toRational(values[1]);
        
        if (!argument || !base)
            throw std::runtime_error("Irrational logs not implemented!");

        double argVal = static_cast<double>(argument->getNumerator()) / argument->getDenominator();
        double baseVal = static_cast<double>(base->getNumerator()) / base->getDenominator();

        if (argVal <= 0 || baseVal <= 0)
            throw std::runtime_error("Logarithm argument and base must be greater than 0.");
        if (baseVal == 1.0)
            throw std::runtime_error("Logarithm base cannot be 1.");

        return NodeFactory::makeNumber(std::log(argVal) / std::log(baseVal));
    }

    NodePtr Logarithm::complexSimplify(OpPtr opNode) const
    {
        const auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");
        
        auto innerOp = Casting::toOperator(opNode->getArgument(0));

        if(innerOp && innerOp->getName() == "^" && innerOp->getArgument(0)->identical(opNode->getArgument(1)))
            return innerOp->getArgument(1);
        
        return opNode->shared_from_this(); 
    }

    NodePtr Logarithm::complexExpansion(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        NodePtr argument = args[0];
        NodePtr base = args[1];

        auto innerOp = Casting::toOperator(argument);
        if(!innerOp)
            return opNode->shared_from_this();
        
        std::vector<NodePtr> newArgs;
        if(innerOp->getName() == "*")
        {
            for(const auto& arg:innerOp->getArguments())
            {
                newArgs.push_back(NodeFactory::makeOperator("log",{arg,base}));
            }
            return OperatorNode::simplify(NodeFactory::makeOperator("+",newArgs));
        }
        if(innerOp->getName() == "^")
        {
            return OperatorNode::simplify(NodeFactory::makeOperator("*",{innerOp->getArgument(1),NodeFactory::makeOperator("log",{innerOp->getArgument(0),opNode->getArgument(1)})}));
        }
        
        return opNode->shared_from_this();
    }


    NodePtr Logarithm::complexLocalSimplify(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        NodePtr argument = args[0];
        NodePtr base = args[1];
        auto innerOp = Casting::toOperator(argument);
        
        // If the internal structures of the argument and base are identical, log_b(b) is exactly 1
        if (argument->identical(base)) {
            return NodeFactory::makeNumber(1);
        }

        // if (auto bNum = Casting::toNumber(base))
        //     if( auto aNum = Casting::toNumber(argument)) {
        //         return evaluate({aNum,bNum},Environment());
        //     }
      
        if(innerOp && innerOp->getName() == "^")
        {
            if(innerOp->getArgument(0)->identical(base))
                return innerOp->getArgument(1); // log(x^b,x) = b
        }

        if (auto num = Casting::toNumber(argument)) {
            if(num->getValue() == 1)
                return NodeFactory::makeNumber(0);
        }


        return opNode->shared_from_this();
    }
    std::string Logarithm::toString(const std::vector<NodePtr> &args) const
    {
        if(args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        bool natural = false;

        if(NumberPtr num = Casting::toNumber(args[1]))
            if(num->getValue() == Config::E)
                natural = true;

        if(natural)
            return "ln("+args[0]->toString()+")";
        else
            return "log(" + args[0]->toString() +"," + args[1]->toString() + ")";
    }

    OpPtr Logarithm::buildInverseOf(OpPtr opToInvert) const
    {
        if(opToInvert->getDefinition()->getName() != getName())
            return nullptr; // not the same operator

        auto& args = opToInvert->getArguments();
        if(args.size() != 2)
            return nullptr; // malformed operator

        NodePtr argument = args[0];
        NodePtr base = args[1];

        return NodeFactory::makeOperator("^",{base,argument}); // inverse of log_a(b) is a^b
    }
}