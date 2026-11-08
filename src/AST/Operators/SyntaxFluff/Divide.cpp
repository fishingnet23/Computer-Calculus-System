#include "../../include/NodeFactory.hpp"
#include "Divide.hpp"
namespace AST
{
OpPtr AST::Divide::clean(const std::vector<NodePtr> &args)
{
    // a/b = a*b^-1
    if(args.size()!=2)
        throw std::runtime_error("Malformed division!");
        
    auto neg1 = NodeFactory::makeNumber(-1);
    auto innerMul = NodeFactory::makeOperator("^",{args[1],neg1});
    return NodeFactory::makeOperator("*",{args[0],innerMul});
}
}