#pragma once
#include "Nodes.hpp"
#include <functional>

namespace AST
{
enum class OpProperty
{
    COMMUTATIVE, // argument order can be changed
    LEFT_ASSOCIATIVE, // grouped left to right
    RIGHT_ASSOCIATIVE, // grouped right to left
    TRUE_ASSOCIATIVE, // nested operations of the same kind can be flattened
};

class OpDefinition
{
protected:
    std::vector<OpProperty> properties;
    // special identites
    virtual NodePtr applySymbolicIdentities(OpPtr opNode) const = 0;
    virtual void simplifyArgsOf(OpPtr opNode) const
    {
        for(auto& arg: opNode->getArguments())
        {
            if(auto innerOp = toOperator(arg))
                arg = innerOp->simplify();
        }

        opNode->unify();
    }
public:
    enum class Format
    {
        NONE = -1,
        INFIX = 0,
        PREFIX = 1,
        POSTFIX = 2,
        EXPLICIT_FUNCTION = 3,
    };

    OpDefinition(){};
    OpDefinition(const std::vector<OpProperty>& properties){this->properties=properties;}

    bool hasProperty(const OpProperty& property) const;
    void addProperty(const OpProperty& property);
    void removeProperty(const OpProperty& property);

    virtual double evaluate(const std::vector<double>& values) const = 0;
    
    // generic simplification and expansion rules of the operator
    virtual NodePtr applySimplification(OpPtr opNode) const = 0;
    virtual NodePtr applyExpansion(OpPtr opNode) const = 0;
    virtual std::string toString(const std::vector<NodePtr>& args) const = 0;
    virtual std::string getName() const = 0;
    virtual Format getFormat() const = 0;
    virtual int getPrecedence() const = 0;

};

class IndexedSymbolDataAccumulator {
public:
    enum class IndexMode {
        ByDegree, 
        ByCoefficient,
    };
    struct indexedOpData
    {
        NodePtr originalNode;
        double uniqueValue;
    };
private:
    
    std::map<std::string, std::map<double,indexedOpData>> map;
    double constant = 0;
    IndexMode mode = IndexMode::ByDegree;
public:
    
    IndexedSymbolDataAccumulator(IndexMode indexingStrategy = IndexMode::ByDegree) : mode(indexingStrategy) {}

    void insertOrAccumulate(const std::string& symbolName, NodePtr node, double coeff, double degree);
    
    void set(const std::string& symbolName, double coeff, double degree);
    const auto& get()const{return map;}
    // get indiscriminantly
    double* getValuePtr(const std::string& symbolName, double primaryKey);
    double* getCoeffPtrByDegree(const std::string& symbolName, double degree);
    double* getDegreePtrByCoeff(const std::string& symbolName, double coeff);

    const std::map<double,indexedOpData>* getRawInternalMap(const std::string& symbolName) const;
    const indexedOpData* getOpData(const std::string& symbolName,double primaryKey) const;
    

    IndexMode getMode() const { return mode; }
    inline double getConstant() const { return constant; }
    inline void setConstant(double c) { constant = c; }
};

class SymbolDataAccumulator {
public:

    struct SymbolData
    {
        NodePtr originalNode;
        double coeff;
        double degree;
    };
private:
    
    std::map<std::string, SymbolData> map; 
    double constant = 0;
public:
    
    SymbolDataAccumulator(){}
    
    SymbolData* getSymbol(const std::string& symbolName);
    const SymbolData* getSymbol(const std::string& symbolName) const;
    
    void insertOrAccumulate(const std::string& symbolName, Symbol* node, const std::function<void(double&,double&)>& transformation);
    void set(const std::string& symbolName, double coeff, double degree);
    const auto& get()const{return map;}
    

    inline double getConstant() const { return constant; }
    inline double& getConstantRef(){ return constant; }
    inline void setConstant(double c) { constant = c; }
};


};