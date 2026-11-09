#pragma once
#include "Config.hpp"
#include "../Structure/OperatorDefinitions.hpp" 
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace AST
{

class NodeFactory
{
public:
    struct RegistryKey
    {
        using NodeType = AST::NodeType;
        NodeType type;
        std::variant<std::monostate, InfRational, std::string> value;
        std::vector<NodePtr> args;
        std::size_t cached_hash; 

        // Operator constructor
        RegistryKey(const std::string& opName, std::vector<NodePtr> arguments)
            : type(NodeType::Operator), value(opName), args(std::move(arguments)) {
            computeHash();
        }

        // Variable constructor
        RegistryKey(NodeType nodeType, const std::string& name)
            : type(nodeType), value(name), args({}) {
            computeHash();
        }

        // Unified Scalar constructor
        RegistryKey(const InfRational& scalarVal)
            : type(NodeType::Scalar), value(scalarVal), args({}) {
            computeHash();
        }
        
        bool operator==(const RegistryKey& other) const {
            // Pure pointer comparisons are now 100% safe because the factory 
            // guarantees global uniqueness down to the leaf nodes.
            if (type != other.type || args != other.args) return false;
            return value == other.value; 
        }



    private:
        void computeHash() {
            std::size_t h = std::hash<int>{}(static_cast<int>(type));
            
            std::visit([&h](const auto& val) {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, std::monostate>) {
                    return;
                } else if constexpr (std::is_same_v<T, std::string>) {
                    h ^= std::hash<std::string>{}(val) + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2);
                } else if constexpr (std::is_same_v<T, InfRational>) {

                    std::string stableStr = val.str();
                    h ^= std::hash<std::string>{}(stableStr) + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2);
                }
            }, value);

            for (const auto& arg : args) {
                std::string argStr = arg->getPreFix();
                h ^= std::hash<std::string>{}(argStr) + 0x9e3779b97f4a7c15 + (h << 6) + (h >> 2);
            }

            cached_hash = h;
        }

    };

    struct RegistryHashFunction {
        std::size_t operator()(const RegistryKey& key) const { return key.cached_hash; }
    };

public:
    static auto& getMap() {
        static std::unordered_map<RegistryKey, WeakNodePtr, RegistryHashFunction> instance;
        return instance;
    }

    // Unified entry points
    static NumberPtr makeNumber(double val);
    static NumberPtr makeNumber(const InfFloat& val);
    static NumberPtr makeNumber(const InfRational& fraction);
    static NumberPtr makeNumber(long long num, long long den);
    static NumberPtr makeNumberFromLiteral(const std::string& literal);


    static VariablePtr makeVariable(const std::string& name);
    static OpPtr makeOperator(const std::string& name, const std::vector<NodePtr>& children);
    static OpPtr getWithArguments(const OperatorNode *op, const std::vector<NodePtr> &children);
    static OpPtr getWithArguments(OpPtr op, const std::vector<NodePtr>& children);

    static NodePtr substitute(NodePtr treeRoot, const Environment&replacementMap);
};
    
}
