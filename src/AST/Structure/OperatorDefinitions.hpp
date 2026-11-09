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

    enum class SortingFormat
    {
        ASCEND,
        DESCEND
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
    virtual SortingFormat getSortingFormat() const {return SortingFormat::ASCEND;}
};

struct SubstitutionRule
{
    NodePtr prerequisteTemplate;
    NodePtr newStructure;
};

class OpDefinition : public SyntaxRule
{
public:

    OpDefinition(){}
    OpDefinition(const std::vector<OpProperty>& properties):SyntaxRule(properties){}
    OpDefinition(const OpDefinition& other):SyntaxRule(other){}
        
    virtual std::shared_ptr<OpDefinition> clone() const = 0;
    
    // simplification rules. As a precondition, the inner arguments must already be simplified. 
    virtual NodePtr simplify(OpPtr opNode) const = 0;
    // expansion rules. As a precondition, the inner arguments must already be expanded.
    virtual NodePtr expand(OpPtr opNode) const = 0;

    // local simplification rules. Does not require fully simplified arguments.
    // this is where structural collapse happens. (Mostly)
    virtual NodePtr localCollapseStructure(OpPtr opNode) const = 0;

    // Factorization rules, rules that map the operator to specifically multiplication
    // This is best used to seperate a side of an equation into sub cases, to exploit the zero product rule,
    // like when you factor the lhs in x^2+x-6 = 0 to (x+3)(x-2) = 0, then the solutions are to the sub equations: x+3=0 and x-2 = 0 (x=-3 or x=2)
    // Not applicable nor necessary for all operators, which is why the default implementation is to just return the input
    virtual NodePtr factorize(OpPtr opNode) const { return opNode;}
    // Approximation rules that don't have to preserve precision.
    virtual InfFloat approximate(const std::vector<InfFloat>& operands) const = 0;

    // builds the inverse of the inputted operator, if possible.
    // If the OpPtr is not invertible, it returns a nullptr.
    // If the OpPtr isn't an operator with this definition, it returns a nullptr.
    virtual OpPtr buildInverseOf(OpPtr opToInvert) const {return nullptr;} // default implementation returns nullptr, meaning the operator is not invertible


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
        InfRational coeff = 0;
        InfRational degree = 0;
    };
private:
    
    std::map<std::string, SymbolData> map; 
    InfRational constant = 0;
public:
    
    SymbolDataAccumulator(){}
    
    SymbolData* getSymbol(const std::string& symbolName);
    const SymbolData* getSymbol(const std::string& symbolName) const;
    
    void insertOrAccumulate(const std::string& symbolName, SymbolPtr node, std::function<void(InfRational&,InfRational&)> transformation,InfRational originalCoeff, InfRational originalExp);
    void set(const std::string& symbolName, InfRational coeff, InfRational degree);
    const std::map<std::string, SymbolData>& get()const{return map;}
    

    inline InfRational getConstant() const { return constant; }
    inline InfRational& getConstantRef(){ return constant; }
    inline void setConstant(InfRational c) { constant = c; }
};


};