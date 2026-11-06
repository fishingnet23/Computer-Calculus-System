#include "../../include/NodeFactory.hpp"
#include "Sin.hpp"

namespace AST
{

Sin::Sin():
    OpDefinition(std::vector<OpProperty>{OpProperty::RIGHT_ASSOCIATIVE}){}

NodePtr Sin::complexLocalSimplify(OpPtr opNode) const
{
    auto& args = opNode->getArguments();
    
    // If the child has collapsed to a pure literal number
    if (auto num = SoftCasting::toNumber(args[0]))
    {
        return evaluate({num},Environment());
    }
    return opNode;
}

std::string Sin::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

NumberPtr Sin::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
{
    if(values.size() != 1)
        throw std::runtime_error("Malformed sin node!");
    
    return NodeFactory::makeNumber(mp::sin(values[0]->getValue())); // allowed to be imprecise
}

NodePtr Sin::complexSimplify(OpPtr opNode) const
{
    auto child = opNode->getArgument(0);
    auto childOp = SoftCasting::toOperator(child);
    
    if (!childOp) return opNode;

   // sin(-x) = -sin(x)
    if (childOp->getName() == "*")
    {
        auto coeff = childOp->getArgument(0);
        if(auto numeric = SoftCasting::toNumber(coeff))
            if(numeric->getValue() < 0)
            {
                auto positiveCoeff = NodeFactory::makeNumber(-numeric->getValue());
                
                std::vector<NodePtr> remainingArgs = childOp->getArguments();
                remainingArgs[0] = positiveCoeff;
                
                auto newInternalMul = OperatorNode::simplify(NodeFactory::makeOperator("*", remainingArgs));
                opNode = NodeFactory::getWithArguments(opNode,std::vector<NodePtr>{newInternalMul});
                auto neg1 = NodeFactory::makeNumber(-1);
                return NodeFactory::makeOperator("*", {neg1, opNode});
            }
    }
    return opNode;
}

NodePtr Sin::complexExpansion(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
