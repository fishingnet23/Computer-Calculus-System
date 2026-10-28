#pragma once
#include "Config.hpp"
#include "Operations.hpp"

namespace AST
{
class NodeFactory
{
public:
    static NodePtr makeNumber(double val);
    static NodePtr makeNumber(const std::string& name);
    static NodePtr makeVariable(const std::string& name);
    static NodePtr makeOperator(const std::string& name, const std::vector<NodePtr>& children);
    static OpPtr makeOperatorPtr(const std::string& name, const std::vector<NodePtr>& children);
    static PlaceholderPtr makePlaceholder(const std::string& name, bool(*predicate)(NodePtr));

    // traverses the tree and looks for nodes with the same string representation as the hashes in the map.
    // when a compatible node is found, it is replaced with the node in the map
    static OpPtr substitute(OpPtr treeToTraverse, const std::map<std::string,NodePtr>& map);

    static NodePtr cloneNode(const NodePtr& original);
};
    
}