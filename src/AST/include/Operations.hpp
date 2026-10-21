#pragma once
#include <map>
#include <string>
#include <memory>
#include "Config.hpp"

#include "../Operators/Core/Add.hpp"
#include "../Operators/Core/Multiply.hpp"
#include "../Operators/Core/Exponentiate.hpp"
#include "../Operators/Trigonometry/Sin.hpp"
#include "../Operators/Trigonometry/Cos.hpp"


#include "../Operators/SyntaxFluff/Divide.hpp"
#include "../Operators/SyntaxFluff/Negate.hpp"
#include "../Operators/SyntaxFluff/Subtract.hpp"

#include "../Operators/General/Max.hpp"
#include "../Operators/General/Min.hpp"

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

        static std::shared_ptr<SyntaxRule> getRule(const std::string& name)
        {
            if(auto op = get(name))
                return op;
            else if(auto fluff = getFluffDefinition(name))
                return fluff;
            else
                return nullptr;
        }

        static OpDefinition::Format getOperationFormat(const std::string& name)
        {
            auto op = getRule(name);
            if(!op)
                return OpDefinition::Format::NONE;
            return op->getFormat();
        }
    };
}
