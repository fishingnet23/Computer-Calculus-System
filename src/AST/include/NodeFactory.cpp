#include "NodeFactory.hpp"

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
    return std::make_shared<VariableNode>(name,1,1);
}

NodePtr NodeFactory::makeVariable(const std::string &name, double coeff, double degree)
{
    return std::make_shared<VariableNode>(name,coeff,degree);
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
NodePtr NodeFactory::cloneNode(const NodePtr &original)
 {
        if (!original) return nullptr;
        return original->clone(); 
    }
}