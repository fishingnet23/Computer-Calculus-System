#include "OperatorDefinitions.hpp"
#include "../include/NodeFactory.hpp"

namespace AST
{

bool AST::OpDefinition::hasProperty(const OpProperty &property) const
{
    for(const auto& prop:properties)
        if(property == prop)
            return true;
    return false;
}
void OpDefinition::addProperty(const OpProperty& property)
    {
        if(hasProperty(property))
            return;
        properties.push_back(property);    
    }
void OpDefinition::removeProperty(const OpProperty& property) {properties.erase(std::remove(properties.begin(), properties.end(), property), properties.end());}

void IndexedSymbolDataAccumulator::insertOrAccumulate(const std::string &symbolName, NodePtr node, double coeff, double degree)
{

    double key = mode == IndexedSymbolDataAccumulator::IndexMode::ByDegree? coeff : degree;
    double value = mode == IndexedSymbolDataAccumulator::IndexMode::ByDegree? coeff : degree;

    auto& slot = map[symbolName][degree];
    slot.uniqueValue += coeff;
    if (!slot.originalNode) {
        slot.originalNode = node;
    }
}

void IndexedSymbolDataAccumulator::set(const std::string &symbolName, double coeff, double degree)
{
    // safe because c++ will automatically create the needed keys and values if they dont exist
    if (mode == IndexMode::ByDegree) {
        map[symbolName][degree].uniqueValue = coeff;
    } else {
        map[symbolName][coeff].uniqueValue = degree;
    }
}
double *IndexedSymbolDataAccumulator::getValuePtr(const std::string &symbolName, double primaryKey)
{
    auto it = map.find(symbolName);
    if (it == map.end()) return nullptr;

    auto innerIt = it->second.find(primaryKey);
    if (innerIt == it->second.end()) return nullptr;

    return &(innerIt->second.uniqueValue);
}

double *IndexedSymbolDataAccumulator::getCoeffPtrByDegree(const std::string &symbolName, double degree)
{
if (mode != IndexMode::ByDegree) return nullptr; // guard against wrong layout assumption
    return getValuePtr(symbolName, degree);
}

double *IndexedSymbolDataAccumulator::getDegreePtrByCoeff(const std::string &symbolName, double coeff)
{
    if (mode != IndexMode::ByCoefficient) return nullptr; // guard against wrong layout assumption
    return getValuePtr(symbolName, coeff);
}
const std::map<double, IndexedSymbolDataAccumulator::indexedOpData> *IndexedSymbolDataAccumulator::getRawInternalMap(const std::string &symbolName) const
{
    auto it = map.find(symbolName);
    if (it == map.end()) return nullptr;
    return &(it->second);
}
const IndexedSymbolDataAccumulator::indexedOpData *IndexedSymbolDataAccumulator::getOpData(const std::string &symbolName, double primaryKey) const
{
     auto it = map.find(symbolName);
    if (it == map.end()) return nullptr;

    auto innerIt = it->second.find(primaryKey);
    if (innerIt == it->second.end()) return nullptr;
    return &(innerIt->second);
}

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

void SymbolDataAccumulator::insertOrAccumulate(const std::string &symbolName, SymbolPtr node, const std::function<void(double &, double &)> &transformation)
{
    insertOrAccumulate(symbolName,node,transformation,1,1);
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
        if(toNumber(originalArgs[i]))
            continue;
        args.push_back(originalArgs[i]);
    }
    return args;
}
}