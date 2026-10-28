#include "NodeFactory.hpp"
#include "StructureManipulation.hpp"

namespace AST{
NodePtr NodeFactory::makeNumber(double val)
{
    return std::make_shared<NumberNode>(val);
}

NodePtr NodeFactory::makeNumber(const std::string &name)
{
    return std::make_shared<NumberNode>(name);
}

NodePtr NodeFactory::makeVariable(const std::string &name)
{
    return std::make_shared<VariableNode>(name);
}

NodePtr NodeFactory::makeOperator(const std::string &name, const std::vector<NodePtr> &children)
{
    return std::static_pointer_cast<Node>(makeOperatorPtr(name,children));
}

OpPtr AST::NodeFactory::makeOperatorPtr(const std::string &name, const std::vector<NodePtr> &children)
{
    auto opDef = OpRegistry::getRule(name); // Assuming an operator registry exists
    if (!opDef) {
        throw std::runtime_error("Unknown operator: " + name);
    }

    if(opDef->getRuleType() == "Operator")
        return std::make_shared<OperatorNode>(children,std::static_pointer_cast<const OpDefinition>(opDef));
    else if(opDef->getRuleType() == "Fluff") // things like division/subtraction
        return std::static_pointer_cast<SyntaxFluff>(opDef)->clean(children);
    else
        throw std::runtime_error("Unknown operator: " + name);
}
PlaceholderPtr NodeFactory::makePlaceholder(const std::string &name, bool (*predicate)(NodePtr))
{
    return std::make_shared<PlaceholderNode>(name,predicate);
}
OpPtr NodeFactory::substitute(OpPtr treeToTraverse, const std::map<std::string, NodePtr> &map)
{
    for(size_t i=0;i<treeToTraverse->getArguments().size();i++)
    {
        auto arg = treeToTraverse->getArgument(i);

        auto it = map.find(arg->toString());
        if(it == map.end())
        {
            // not found, look nested
            auto op = Casting::toOperator(arg);
            if(op)
            {
                OpPtr res = substitute(op,map); // recursive descent
                treeToTraverse->getArguments()[i] = res; // save new version
                continue;
            }
            continue; // next argument
        }

        //found
        NodePtr mappedNode = it->second;
        treeToTraverse->getArguments()[i] = mappedNode; // replace 
    }
    return treeToTraverse;
}
NodePtr NodeFactory::cloneNode(const NodePtr &original)
{
    if (!original)
        return nullptr;
    return original->clone();
}
}