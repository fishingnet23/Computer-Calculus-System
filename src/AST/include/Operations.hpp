#pragma once
#include <map>
#include <string>
#include <memory>
#include "../Operators/Add.hpp"
#include "../Operators/Multiply.hpp"
#include "../Operators/Exponentiate.hpp"

#include "../Operators/Divide.hpp"

namespace AST
{
    class OpRegistry
    {
    private:
        static const std::map<std::string, std::shared_ptr<OpDefinition>> map;
        static const std::map<std::string, std::shared_ptr<SyntaxFluff>> fluffyMap;
    public:
        static std::shared_ptr<OpDefinition> get(const std::string& name)
        {
            auto it = map.find(name);
            if (it != map.end()) {
                return it->second;
            }
            return nullptr;
        }

        static std::shared_ptr<SyntaxFluff> getFluffDefinition(const std::string& name)
        {
            auto it = fluffyMap.find(name);
            if (it != fluffyMap.end()) {
                return it->second;
            }
            return nullptr;
        }

        static OpDefinition::Format getOperationFormat(const std::string& name)
        {
            auto op = get(name);
            if(!op)
                return OpDefinition::Format::NONE;
            return op->getFormat();
        }
    };
}
