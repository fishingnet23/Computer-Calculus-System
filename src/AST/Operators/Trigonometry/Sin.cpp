#include "../../include/NodeFactory.hpp"
#include "Sin.hpp"

namespace AST
{

NodePtr Sin::localSimplification(OpPtr opNode) const
{
    auto& args = opNode->getArguments();
    
    // If the child has collapsed to a pure literal number
    if (auto num = toNumber(args[0]))
    {
        return NodeFactory::makeNumber(evaluate({num->getValue()}));
    }
    return opNode;
}

std::string Sin::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

double Sin::evaluate(const std::vector<double> &values) const
{
    if(values.size() != 1)
        throw std::runtime_error("Malformed sin node!");
    return std::sin(values[0]);
}

NodePtr Sin::applySimplification(OpPtr opNode) const
{
    auto child = opNode->getArgument(0);
    auto childOp = toOperator(child);
    
    if (!childOp) return opNode;

   // sin(-x) = -sin(x)
    if (childOp->getName() == "*")
    {
        auto coeff = childOp->getArgument(0);
        if(auto numeric = toNumber(coeff))
            if(numeric->getValue() < 0)
            {
                auto positiveCoeff = NodeFactory::makeNumber(-numeric->getValue());
                
                std::vector<NodePtr> remainingArgs = childOp->getArguments();
                remainingArgs[0] = positiveCoeff;
                
                auto newInternalMul = NodeFactory::makeOperatorPtr("*", remainingArgs)->simplify();
                opNode->setArguments({newInternalMul});
                auto neg1 = NodeFactory::makeNumber(-1);
                return NodeFactory::makeOperator("*", {neg1, opNode});
            }
    }
    return opNode;
}

NodePtr Sin::applyExpansion(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
