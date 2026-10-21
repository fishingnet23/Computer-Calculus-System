#pragma once
#include "Config.hpp"
#include "../include/Operations.hpp"

namespace AST
{
class NodeFactory
{
public:
    static NodePtr makeNumber(double val) {
        return std::make_shared<NumberNode>(val);
    }
    static NodePtr makeNumber(const std::string& name) {
        return std::make_shared<NumberNode>(name);
    }
    
    static NodePtr makeVariable(const std::string& name) {
        return std::make_shared<VariableNode>(name,1,1);
    }
    static NodePtr makeVariable(const std::string& name, double coeff, double degree) {
        return std::make_shared<VariableNode>(name,coeff,degree);
    }

    static NodePtr makeOperator(const std::string& name, const std::vector<NodePtr>& children) {
        return std::static_pointer_cast<Node>(makeOperatorPtr(name,children));
    }
    static OpPtr makeOperatorPtr(const std::string& name, const std::vector<NodePtr>& children) {
        auto opDef = OpRegistry::get(name); // Assuming an operator registry exists
        if (!opDef) {
            throw std::runtime_error("Unknown operator: " + name);
        }
        return std::make_shared<OperatorNode>(children, opDef);
    }

    static NodePtr cloneNode(const NodePtr& original) {
        if (!original) return nullptr;
        return original->clone(); 
    }
};
    
}