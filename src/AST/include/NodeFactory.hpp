#pragma once
#include "Config.hpp"
#include "../include/Operations.hpp"

namespace AST
{
class NodeFactory
{
public:
    static NodePtr makeNumber(double val);
    static NodePtr makeNumber(const std::string& name);
    static NodePtr makeVariable(const std::string& name);
    static NodePtr makeVariable(const std::string& name, double coeff, double degree);
    static NodePtr makeOperator(const std::string& name, const std::vector<NodePtr>& children);
    static OpPtr makeOperatorPtr(const std::string& name, const std::vector<NodePtr>& children);

    static NodePtr cloneNode(const NodePtr& original);
};
    
}