#include "../../include/NodeFactory.hpp"
#include "Cos.hpp"

namespace AST
{
Cos::Cos():
    OpDefinition(std::vector<OpProperty>{OpProperty::RIGHT_ASSOCIATIVE}){}

NodePtr Cos::complexLocalSimplify(OpPtr opNode) const
{
    auto& args = opNode->getArguments();
    
    // If the child has collapsed to a pure literal number
    if (auto num = Casting::toNumber(args[0]))
    {
        return evaluate({num},Environment());
    }
    return opNode;
}

std::string Cos::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

NumberPtr Cos::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
{
    if(values.size() != 1)
        throw std::runtime_error("Malformed cos node!");
    return NodeFactory::makeNumber(std::cos(values[0]->getValue()));
}

NodePtr Cos::complexSimplify(OpPtr opNode) const
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
                
                auto newInternalMul = OperatorNode::simplify(NodeFactory::makeOperator("*", remainingArgs));
                opNode = NodeFactory::getWithArguments(opNode,std::vector<NodePtr>{newInternalMul});
            }
    }
    return opNode;
}

NodePtr Cos::complexExpansion(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
