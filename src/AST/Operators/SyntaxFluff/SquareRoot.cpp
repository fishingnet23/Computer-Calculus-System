#include "SquareRoot.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::SquareRoot::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed sqrt!");
    return NodeFactory::makeOperatorPtr("^",{args[0], NodeFactory::makeNumber("0.5")});
}
}