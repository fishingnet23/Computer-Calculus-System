#include "../../include/NodeFactory.hpp"
#include "Logarithm.hpp"

namespace AST
{
    Logarithm::Logarithm():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

    InfFloat Logarithm::approximate(const std::vector<InfFloat> &operands) const
    {
        if(operands.size()!=2)
            throw std::runtime_error("Malformed logarithm!");

        return mp::log(operands[0]) / mp::log(operands[1]);
    }

    NodePtr Logarithm::simplify(OpPtr opNode) const
    {
        const auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");
        
        auto innerOp = SoftCasting::toOperator(opNode->getArgument(0));

        if(innerOp && innerOp->getName() == "^" && innerOp->getArgument(0) == (opNode->getArgument(1)))
            return innerOp->getArgument(1);
        
        return opNode->shared_from_this(); 
    }

    NodePtr Logarithm::expand(OpPtr opNode) const
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


    NodePtr Logarithm::localCollapseStructure(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        NodePtr argument = args[0];
        NodePtr base = args[1];
        auto innerOp = SoftCasting::toOperator(argument);
        
        // If the internal structures of the argument and base are identical, log_b(b) is exactly 1
        if (argument == base) {
            return NodeFactory::makeNumber(1);
        }

        // if (auto bNum = Casting::toNumber(base))
        //     if( auto aNum = Casting::toNumber(argument)) {
        //         return evaluate({aNum,bNum},Environment());
        //     }
      
        if(innerOp && innerOp->getName() == "^")
        {
            if(innerOp->getArgument(0) == base)
                return innerOp->getArgument(1); // log(x^b,x) = b
        }

        if (auto num = SoftCasting::toRawNumber(argument)) {
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
            if(SymbolPtr sym = SoftCasting::toSymbol(args[1]))
                if(sym->getName() == "e")
                    natural = true;

        if(natural)
            return "ln("+args[0]->toString()+")";
        else
            return "log(" + args[0]->toString() +"," + args[1]->toString() + ")";
    }
}