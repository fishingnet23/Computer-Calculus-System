#include "../../include/NodeFactory.hpp"
#include "Negate.hpp"
namespace AST
{
OpPtr AST::Negate::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed negation!");
    auto neg1 = NodeFactory::makeNumber(-1);
    return NodeFactory::makeOperatorPtr("*",{neg1,args[0]});
}
}