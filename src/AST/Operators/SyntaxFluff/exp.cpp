#include "exp.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::Exp::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed exp!");
    return NodeFactory::makeOperatorPtr("^",{NodeFactory::makeNumber(E),args[0]});
}
}