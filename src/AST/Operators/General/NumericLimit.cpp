#include "../../include/NodeFactory.hpp"
#include "NumericLimit.hpp"

namespace AST
{

NodePtr NumericLimit::localCollapseStructure(OpPtr opNode) const
{
    const auto& args = opNode->getArguments();
    
    // If the children have collapsed to all numbers
    size_t numbers = 0;
    for(const auto& arg:args)
        if( const auto& numeric = SoftCasting::toNumber(arg))
            numbers++;
    if(numbers == args.size())
    {
        NumberPtr newVal = SoftCasting::toNumber(args[0]);
        for(const auto& val:args)
            newVal = evaluationFn(newVal,SoftCasting::toNumber(val)); //explicitly safe 
        return newVal;
    }
    return opNode;
}

InfFloat NumericLimit::approximate(const std::vector<InfFloat> &operands) const
{
    auto newVal = operands[0];
    for(const auto& val:operands)
        newVal = approximationFn(newVal,val); //explicitly safe 
    return newVal;
}

std::string NumericLimit::toString(const std::vector<NodePtr> &args) const
{
    if(args.size() < 2)
        throw std::runtime_error("Malformed " + getName() + "! Can't build a string!");
    std::string str =  getName() + "(" + args[0]->toString();

    for(size_t i=1;i<args.size();i++)
    {
        str+= "," + args[i]->toString();
    }
    return str + ")";
}

NodePtr NumericLimit::simplify(OpPtr opNode) const
{
    return opNode->shared_from_this();
}

NodePtr NumericLimit::expand(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
