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

    // The function is a mapping of functions, not numbers
    // This means it is a transformation of entire functions, not a raw operation
    // A good example is the derivative.
    FUNCTIONAL_MAPPING 
    
};

class SyntaxRule
{
protected:
    std::vector<OpProperty> properties;
public:

    enum class Format
    {
        NONE = -1,
        INFIX = 0,
        PREFIX = 1,
        POSTFIX = 2,
        EXPLICIT_FUNCTION = 3,
    };

    SyntaxRule(){};
    SyntaxRule(const std::vector<OpProperty>& properties){this->properties=properties;}
    SyntaxRule( const SyntaxRule& other): properties(other.properties){}

    bool hasProperty(const OpProperty& property) const;
    void addProperty(const OpProperty& property);
    void removeProperty(const OpProperty& property);
   
    virtual std::string toString(const std::vector<NodePtr>& args) const = 0;
    virtual std::string getName() const = 0;
    virtual Format getFormat() const = 0;
    virtual size_t getPrecedence() const = 0;
    virtual size_t getCanonArgumentCount() const = 0;

    virtual std::string getRuleType() const = 0;
};

class OpDefinition : public SyntaxRule
{

public:

    OpDefinition(){};
    OpDefinition(const std::vector<OpProperty>& properties):SyntaxRule(properties){}
    OpDefinition( const OpDefinition& other): SyntaxRule(other){}
    virtual std::shared_ptr<OpDefinition> clone() const = 0;

    virtual double evaluate(const std::vector<double>& values, const Environment& env)const = 0;
    virtual double evaluate(const std::vector<double>& values) const {return evaluate(values,Environment());}

    
    // generic simplification and expansion rules of the operator
    virtual NodePtr applySimplification(OpPtr opNode) const = 0;
    virtual NodePtr applyExpansion(OpPtr opNode) const = 0;

    virtual NodePtr localSimplification(OpPtr opNode) const = 0;


    static std::vector<NodePtr> reduceToSymbols(const OpPtr& op);

    virtual std::string getRuleType() const override {return "Operator";}
};

class SyntaxFluff : public SyntaxRule // Represents the simplification rule of an operation like subtraction
{
public:

    SyntaxFluff(){};
    SyntaxFluff(const std::vector<OpProperty>& properties):SyntaxRule(properties){}
    SyntaxFluff( const SyntaxFluff& other): SyntaxRule(other){}

    virtual OpPtr clean(const std::vector<NodePtr>& args) = 0; // builds an Operator by applying its transformation to the arguments

    virtual std::string getRuleType() const override {return "Fluff";}
};

class SymbolDataAccumulator {
public:

    struct SymbolData
    {
        SymbolPtr originalNode;
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
    
    void insertOrAccumulate(const std::string& symbolName, SymbolPtr node, const std::function<void(double&,double&)>& transformation,double originalCoeff, double originalExp);
    void set(const std::string& symbolName, double coeff, double degree);
    const std::map<std::string, SymbolData>& get()const{return map;}
    

    inline double getConstant() const { return constant; }
    inline double& getConstantRef(){ return constant; }
    inline void setConstant(double c) { constant = c; }
};


};