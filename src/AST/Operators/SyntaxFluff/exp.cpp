#include "exp.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::Exp::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed exp!");
    return NodeFactory::makeOperator("^",{NodeFactory::makeNumberFromLiteral("e"),args[0]});
}
}