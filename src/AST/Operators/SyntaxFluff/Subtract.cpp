#include "../../include/NodeFactory.hpp"
#include "Subtract.hpp"
namespace AST
{
OpPtr Subtract::clean(const std::vector<NodePtr> &args)
{
    // a-b = a+(-1*b)
    if(args.size()!=2)
        throw std::runtime_error("Malformed subtraction!");
    auto neg1 = NodeFactory::makeNumber(-1);
    auto innerMul = NodeFactory::makeOperator("*",{neg1,args[1]});
    return NodeFactory::makeOperator("+",{args[0],innerMul});
}
}