#include "NodeFactory.hpp"
#include "Operations.hpp"
#include <stdexcept>
#include <numeric>
#include <cmath>

namespace AST {

static RationalNode rationalApproximation(double val)
{
    double leftToApproximate = val;

    long long olderNumerator = 0, previousNumerator = 1;
    long long olderDenominator = 1, previousDenominator = 0;

    RationalNode r;

    while(true)
    {
        long long remainder = std::floor(leftToApproximate);
        
        long long numerator = remainder * previousNumerator + olderNumerator;
        long long denominator = remainder * previousDenominator + olderDenominator;
        
        if(denominator > RationalNode::MAX_DENOMINATOR)
            break;

        r = RationalNode(numerator,denominator);
        
        if (std::abs(leftToApproximate - remainder) < Config::EPSILON){
            break;
        }
        leftToApproximate = 1.0/(leftToApproximate-remainder);

        olderNumerator = previousNumerator;
        previousNumerator = numerator;

        olderDenominator = previousDenominator;
        previousDenominator = denominator;

       
    }

    return r;
}

NumberPtr NodeFactory::makeNumber(double val)
{
    RationalNode r = rationalApproximation(val);
    return makeNumber(r.getNumerator(),r.getDenominator());
}

NumberPtr NodeFactory::makeNumber(long long num, long long den)
{
    RationalNode::reduceRational(num,den);
    RegistryKey key{"RationalNode:" + std::to_string(num) + "/" + std::to_string(den)};
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto rat = Casting::toRational(cached))
                return rat;
        }
    }

    auto newNode = std::make_shared<const RationalNode>(num,den);
    it->second = newNode;

    return newNode;
}

NumberPtr NodeFactory::makeNumberFromLiteral(const std::string &name)
{
    return makeNumber(std::stod(name));
}

VariablePtr NodeFactory::makeVariable(const std::string &name)
{
    RegistryKey key{"VariableNode:" + name};
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto x = Casting::toVariable(cached))
                return x;
        }
    }

    auto newNode = std::make_shared<const VariableNode>(name);
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

    RegistryKey key{"OperatorNode:"+name, children};
    auto [it, inserted] = getMap().try_emplace(key, std::weak_ptr<const Node>{});
    
    if (!inserted) {
        if (auto cached = it->second.lock()) {
            if(auto op = Casting::toOperator(cached))
                return op;
        }
    }

    auto newNode = std::make_shared<const OperatorNode>(children, std::static_pointer_cast<const OpDefinition>(opDef));
    it->second = newNode;

    return newNode;
}

NodePtr NodeFactory::substitute(NodePtr treeRoot, const std::map<std::string, NodePtr>&replacementMap)
{

    auto treeToTraverse = Casting::toOperator(treeRoot);
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

        auto op = Casting::toOperator(arg);
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
