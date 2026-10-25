#include "NaturalLog.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
OpPtr AST::NaturalLog::clean(const std::vector<NodePtr> &args)
{
    if(args.size()!=1)
        throw std::runtime_error("Malformed ln!");
    return NodeFactory::makeOperatorPtr("log",{args[0],NodeFactory::makeNumber(E)});
}
}