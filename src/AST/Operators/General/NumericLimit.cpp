#include "../../include/NodeFactory.hpp"
#include "NumericLimit.hpp"

namespace AST
{

NodePtr NumericLimit::localSimplification(OpPtr opNode) const
{
    const auto& args = opNode->getArguments();
    
    // If the children have collapsed to all numbers
    size_t numbers = 0;
    for(const auto& arg:args)
        if( const auto& numeric = toNumber(arg))
            numbers++;
    if(numbers == args.size())
        return NodeFactory::makeNumber(opNode->evaluate(Environment()));
    return opNode;
}

std::string NumericLimit::toString(const std::vector<NodePtr> &args) const
{
    if(args.size() < 2)
        throw std::runtime_error("Malformed " + getName() + "! Can't build a string!");
    std::string str =  getName() + "(" + args[0]->toString();

    for(size_t i=1;i<args.size();i++)
    {
        str+= ", " + args[i]->toString();
    }
    return str + ")";
}

double NumericLimit::evaluate(const std::vector<double> &values) const
{
    if(values.size() < 2)
        throw std::runtime_error("Malformed "+getName() +"! Can't evaluate!");

    double newVal = values[0];
    for(const auto& val:values)
        newVal = evaluationFn(newVal,val);
    return newVal;
}

NodePtr NumericLimit::applySimplification(OpPtr opNode) const
{
    return opNode->shared_from_this();
}

NodePtr NumericLimit::applyExpansion(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
