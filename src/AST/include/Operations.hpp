#pragma once
#include <map>
#include <string>
#include <memory>
#include "Config.hpp"
#include "../Structure/OperatorDefinitions.hpp"

namespace AST
{
    class OpRegistry
    {

    private:
         static auto& getMutableMap() {
            static std::map<std::string, std::shared_ptr<OpDefinition>> operations;
            return operations;
        }

        static auto& getMutableFluffyMap() {
            static std::map<std::string, std::shared_ptr<SyntaxFluff>> operations;
            return operations;
        }
    
    public:
        static const auto& getMap() { return getMutableMap(); }
        static const auto& getFluffyMap() { return getMutableFluffyMap();}

        static void initialize();

        static std::shared_ptr<OpDefinition> get(const std::string& name)
        {
            OpRegistry::initialize(); 

            auto& map = getMap();
            auto it = map.find(name);
            if (it != map.end()) {
                return it->second;
            }
            return nullptr;
        }

        static std::shared_ptr<SyntaxFluff> getFluffDefinition(const std::string& name)
        {
            OpRegistry::initialize(); 

            auto& fluffyMap = getFluffyMap();
            auto it = fluffyMap.find(name);
            if (it != fluffyMap.end()) {
                return it->second;
            }
            return nullptr;
        }

        static std::shared_ptr<SyntaxRule> getRule(const std::string& name)
        {
            OpRegistry::initialize(); 

            if(auto op = get(name))
                return op;
            else if(auto fluff = getFluffDefinition(name))
                return fluff;
            else
                return nullptr;
        }

        static OpDefinition::Format getOperationFormat(const std::string& name)
        {
            OpRegistry::initialize(); 

            auto op = getRule(name);
            if(!op)
                return OpDefinition::Format::NONE;
            return op->getFormat();
        }
    };
}
