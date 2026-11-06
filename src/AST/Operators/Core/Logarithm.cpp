#include "../../include/NodeFactory.hpp"
#include "Logarithm.hpp"

namespace AST
{
    Logarithm::Logarithm():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    NumberPtr Logarithm::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        if(values.size() != 2)
            throw std::runtime_error("Malformed exponentiation!");

        
        InfFloat baseFloat, argFloat;
        if (auto base = SoftCasting::toRational(values[0]))
        {
            baseFloat = base->getFraction().convert_to<InfFloat>();
        }
        else if(auto base = SoftCasting::toFloat(values[0]))
        {
            baseFloat = base->getFloat();
        }
        else
            throw std::runtime_error("Invalid scalar base for logarithm!");

        if (auto arg = SoftCasting::toRational(values[1]))
        {
            argFloat = arg->getFraction().convert_to<InfFloat>();
        }
        else if(auto arg = SoftCasting::toFloat(values[1]))
        {
            argFloat = arg->getFloat();
        }
        else
            throw std::runtime_error("Invalid scalar argument for logarithm!");

        InfFloat finalResult = mp::log(argFloat) / mp::log(baseFloat);

        return NodeFactory::makeNumber(finalResult); 
    }

    NodePtr Logarithm::complexSimplify(OpPtr opNode) const
    {
        const auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");
        
        auto innerOp = SoftCasting::toOperator(opNode->getArgument(0));

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

        auto innerOp = SoftCasting::toOperator(argument);
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
        auto innerOp = SoftCasting::toOperator(argument);
        
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

        if (auto num = SoftCasting::toNumber(argument)) {
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

        if(NumberPtr num = SoftCasting::toNumber(args[1]))
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