#include "../../include/NodeFactory.hpp"
#include "Cos.hpp"

namespace AST
{

NodePtr Cos::localSimplification(OpPtr opNode) const
{
    auto& args = opNode->getArguments();
    
    // If the child has collapsed to a pure literal number
    if (auto num = Casting::toNumber(args[0]))
    {
        return NodeFactory::makeNumber(evaluate({num->getValue()}));
    }
    return opNode;
}

std::string Cos::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

double Cos::evaluate(const std::vector<double> &values) const
{
    if(values.size() != 1)
        throw std::runtime_error("Malformed sin node!");
    return std::cos(values[0]);
}

NodePtr Cos::applySimplification(OpPtr opNode) const
{
    auto child = opNode->getArgument(0);
    auto childOp = Casting::toOperator(child);
    
    if (!childOp) return opNode;

   // cos(-x) = cos(x)
    if (childOp->getName() == "*")
    {
        auto coeff = childOp->getArgument(0);
        if(auto numeric = Casting::toNumber(coeff))
            if(numeric->getValue() < 0)
            {
                auto positiveCoeff = NodeFactory::makeNumber(-numeric->getValue());
                
                std::vector<NodePtr> remainingArgs = childOp->getArguments();
                remainingArgs[0] = positiveCoeff;
                
                auto newInternalMul = NodeFactory::makeOperatorPtr("*", remainingArgs)->simplify();

                opNode->setArguments({newInternalMul});
            }
    }
    return opNode;
}

NodePtr Cos::applyExpansion(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
