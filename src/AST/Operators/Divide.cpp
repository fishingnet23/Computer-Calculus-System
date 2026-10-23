#include "../include/NodeFactory.hpp"
#include "Divide.hpp"
namespace AST
{
OpPtr AST::Divide::clean(std::vector<NodePtr> &args)
{
    if(args.size()!=2)
        throw std::runtime_error("Malformed division!");
    auto neg1 = NodeFactory::makeNumber(-1);
    auto innerMul = NodeFactory::makeOperator("*",{neg1,args[1]});
    return NodeFactory::makeOperatorPtr("*",{args[0],innerMul});
}
}