#include "ImplicitMul.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::ImplicitMul::clean(const std::vector<NodePtr> &args)
{
    return NodeFactory::makeOperator("*",args);
}
}