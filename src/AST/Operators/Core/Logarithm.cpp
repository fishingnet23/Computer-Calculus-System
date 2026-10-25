#include "../../include/NodeFactory.hpp"
#include "Logarithm.hpp"

namespace AST
{
    double Logarithm::evaluate(const std::vector<double>& values, const Environment& env) const
    {
        if(values.size()!= 2)
            throw std::runtime_error("Malformed logarithm!");
        return std::log(values[0]) / std::log(values[1]);
    }
    NodePtr Logarithm::applySimplification(OpPtr opNode) const
    {

        
        return opNode->shared_from_this(); 
    }

    NodePtr Logarithm::applyExpansion(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        NodePtr argument = args[0];
        NodePtr base = args[1];

        
        return opNode->shared_from_this();
    }


    NodePtr Logarithm::localSimplification(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();
        if (args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        NodePtr argument = args[0];
        NodePtr base = args[1];

        // If the internal structures of the argument and base are identical, log_b(b) is exactly 1
        if (argument->identical(base)) {
            return NodeFactory::makeNumber(1);
        }


        return opNode->shared_from_this();
    }
    std::string Logarithm::toString(const std::vector<NodePtr> &args) const
    {
        if(args.size() != 2)
            throw std::runtime_error("Malformed logarithm!");

        bool natural = false;

        if(NumberPtr num = Casting::toNumber(args[1]))
            if(num->getValue() == E)
                natural = true;

        if(natural)
            return "ln("+args[0]->toString()+")";
        else
            return "log(" + args[0]->toString() +", " + args[1]->toString() + ")";
    }
}