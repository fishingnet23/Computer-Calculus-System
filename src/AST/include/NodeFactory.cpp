#include "NodeFactory.hpp"
#include "Operations.hpp"
#include <stdexcept>
#include <numeric>
#include <cmath>

namespace AST {

// fix to massive bug that caused weak pointers to not get cleaned from the map
template<typename NodeType, typename... Args>
std::shared_ptr<const NodeType> makeTrackedNode(const NodeFactory::RegistryKey& key, Args&&... args) {
    auto* rawPtr = new NodeType(std::forward<Args>(args)...);

    return std::shared_ptr<const NodeType>(rawPtr, [key](const NodeType* ptr) {
        delete ptr;
        auto& map = NodeFactory::getMap();
        auto it = map.find(key);
        if (it != map.end() && it->second.expired()) {
            map.erase(it);
        }
    });
}

NumberPtr NodeFactory::makeNumber(double val)
{
    return makeNumber(InfRational(val));
}

NumberPtr NodeFactory::makeNumber(const InfFloat &val) {
    if (val == 0.0) {
        return makeNumber(InfRational(0, 1));
    }

    try {
        InfFloat rounded = mp::round(val);
        if (mp::abs(val - rounded) < 1e-30) {
            return makeNumber(InfRational(rounded.convert_to<mp::cpp_int>(), 1));
        }

        int exp;
        InfFloat mantissa = mp::frexp(val, &exp);

  
        const int precision_bits = 128; 
        mp::cpp_int scale_factor = mp::pow(mp::cpp_int(2), precision_bits);
        
        mp::cpp_int num = (mantissa * InfFloat(scale_factor)).convert_to<mp::cpp_int>();
        mp::cpp_int den = scale_factor;

        if (exp > 0) {
            num <<= exp;
        } else if (exp < 0) {
            den <<= (-exp);
        }

        InfRational canonicalFraction(num, den); 
        return makeNumber(canonicalFraction);
    }
    catch (...) {
        // Fallback safety barrier for extreme corner cases or numeric overflows
        throw std::runtime_error("CRITICAL: Failed to convert InfFloat to exact canonical rational representation.");
    }
}

NumberPtr NodeFactory::makeNumber(const InfRational &fraction)
{
    RegistryKey key(fraction);
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto num = SoftCasting::toNumber(cached))
                return num;
        }
    }

    auto newNode = makeTrackedNode<RationalNode>(key, fraction);
    it->second = newNode;

    return newNode;
}

NumberPtr NodeFactory::makeNumber(long long num, long long den)
{
    InfRational r(num,den);
    return makeNumber(r);
}

NumberPtr NodeFactory::makeNumberFromLiteral(const std::string &literal)
{
    return makeNumber(InfRational(literal));
}

VariablePtr NodeFactory::makeVariable(const std::string &name)
{
    RegistryKey key(RegistryKey::NodeType::Variable, name);
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto x = SoftCasting::toVariable(cached))
                return x;
        }
    }

    auto newNode = makeTrackedNode<VariableNode>(key, name);
    it->second = newNode;

    return newNode;
}

OpPtr NodeFactory::makeOperator(const std::string &name, const std::vector<NodePtr> &children)
{
    auto opDef = OpRegistry::getRule(name);
    if (!opDef) {
        throw std::runtime_error("Unknown operator: " + name);
    }

    if (opDef->getRuleType() == "Fluff")
        return std::static_pointer_cast<SyntaxFluff>(opDef)->clean(children);

    RegistryKey key(name, children);
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto op = SoftCasting::toOperator(cached))
                return op;
        }
    }

    auto newNode = makeTrackedNode<OperatorNode>(key, children, std::static_pointer_cast<const OpDefinition>(opDef));
    it->second = newNode;

    return newNode;
}

NodePtr NodeFactory::substitute(NodePtr treeRoot, const std::map<std::string, NodePtr>&replacementMap)
{
    auto treeToTraverse = SoftCasting::toOperator(treeRoot);
    if(!treeToTraverse)
    {
        auto it = replacementMap.find(treeRoot->toString());
        if (it != replacementMap.end())
        {
            return it->second;
        }
        return treeRoot;
    }

    const auto& args = treeToTraverse->getArguments();
    std::vector<NodePtr> newArgs;
    newArgs.reserve(args.size());

    for (size_t i = 0; i < args.size(); i++)
    {
        auto arg = treeToTraverse->getArgument(i);

        auto it = replacementMap.find(arg->toString());
        if (it != replacementMap.end())
        {
            newArgs.push_back(it->second);
            continue;
        }

        auto op = SoftCasting::toOperator(arg);
        if (op)
        {
            NodePtr res = substitute(op, replacementMap); 
            newArgs.push_back(res);
        }
        else
        {
            newArgs.push_back(arg); 
        }
    }
    return NodeFactory::getWithArguments(treeToTraverse, newArgs);
}

OpPtr NodeFactory::getWithArguments(const OperatorNode *op, const std::vector<NodePtr> &children)
{
    return NodeFactory::makeOperator(op->getName(), children);
}

OpPtr NodeFactory::getWithArguments(OpPtr op, const std::vector<NodePtr> &children)
{
    return getWithArguments(op.get(), children);
}

}
