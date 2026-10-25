#include "OperatorDefinitions.hpp"
#include "../include/NodeFactory.hpp"

namespace AST
{

bool AST::SyntaxRule::hasProperty(const OpProperty &property) const
{
    for(const auto& prop:properties)
        if(property == prop)
            return true;
    return false;
}
void SyntaxRule::addProperty(const OpProperty& property)
    {
        if(hasProperty(property))
            return;
        properties.push_back(property);    
    }
void SyntaxRule::removeProperty(const OpProperty& property) {properties.erase(std::remove(properties.begin(), properties.end(), property), properties.end());}

SymbolDataAccumulator::SymbolData *SymbolDataAccumulator::getSymbol(const std::string &symbolName)
{
    auto it = map.find(symbolName);
    if (it == map.end()) return nullptr;
    return &(it->second);
}
const SymbolDataAccumulator::SymbolData *SymbolDataAccumulator::getSymbol(const std::string &symbolName) const
{
    auto it = map.find(symbolName);
    if (it == map.end()) return nullptr;
    return &(it->second);
}
void SymbolDataAccumulator::insertOrAccumulate(const std::string &symbolName, SymbolPtr node, const std::function<void(double &, double &)> &transformation, double originalCoeff, double originalExp)
{
    auto& data = map[symbolName];

    if(!data.originalNode)
    {
        data.originalNode = node;
        data.coeff = originalCoeff;
        data.degree = originalExp;
        return;
    }

    transformation(data.coeff,data.degree);
}
void SymbolDataAccumulator::set(const std::string &symbolName, double coeff, double degree)
{
    SymbolData* data = getSymbol(symbolName);
    if(!data)
        return;

    data->coeff=coeff;
    data->degree = degree;
}
std::vector<NodePtr> OpDefinition::reduceToSymbols(const OpPtr &op)
{
    const std::vector<NodePtr>& originalArgs = op->getArguments();
    std::vector<NodePtr> args;
    args.reserve(originalArgs.size());
    for(size_t i=0;i<originalArgs.size();i++)
    {
        if(Casting::toNumber(originalArgs[i]))
            continue;
        args.push_back(originalArgs[i]);
    }
    return args;
}
}