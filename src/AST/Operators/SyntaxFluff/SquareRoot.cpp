#include "SquareRoot.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::SquareRoot::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed sqrt!");
    return NodeFactory::makeOperator("^",{args[0], NodeFactory::makeNumber(1,2)});
}
}