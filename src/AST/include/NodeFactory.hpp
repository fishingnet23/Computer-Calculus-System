#pragma once
#include "Config.hpp"
#include "../Structure/OperatorDefinitions.hpp" 
namespace AST
{


class NodeFactory
{
    struct RegistryKey
    {
        std::string name;
        std::vector<NodePtr> args;
        std::size_t cached_hash; 
        
        RegistryKey(const std::string& name, std::vector<NodePtr> args):name(name),args(args){
            std::size_t h = std::hash<std::string>{}(name);
            for (const auto& arg : args) {
                h ^= std::hash<NodePtr>{}(arg) + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2);
            }
            cached_hash = h;
        }

        RegistryKey(const std::string& name):name(name),args({}){
            cached_hash = std::hash<std::string>{}(name);
        }
        
        bool operator==(const RegistryKey& other) const {return name == other.name && args == other.args;}
    };
    struct RegistryHashFunction {
        std::size_t operator()(const RegistryKey& key) const {
            return key.cached_hash; 
        }
    };

    // to ensure the map is accessible even from static methods that are called on start up, it's wrapped in a function
    static auto& getMap() {
        static std::unordered_map<RegistryKey, WeakNodePtr, RegistryHashFunction> instance;
        return instance;
    }


public:


    static NumberPtr makeNumber(double val);
    static NumberPtr makeNumber(long long numerator, long long denominator);
    static NumberPtr makeNumberFromLiteral(const std::string& name);
    static VariablePtr makeVariable(const std::string& name);
    static OpPtr makeOperator(const std::string& name, const std::vector<NodePtr>& children);

    static OpPtr getWithArguments(const OperatorNode* op, const std::vector<NodePtr>& children);
    static OpPtr getWithArguments(OpPtr op, const std::vector<NodePtr>& children);

    // traverses the tree and looks for nodes with the same string representation as the hashes in the map.
    // when a compatible node is found, it is replaced with the node in the map
    static NodePtr substitute(NodePtr treeRoot, const std::map<std::string,NodePtr>& map);

    

};
    
}