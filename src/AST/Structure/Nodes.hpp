#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
namespace AST
{

class Node;

class NumberNode;
class RationalNode;
class DecimalNode;

class Symbol;
class VariableNode;

class OperatorNode;
class OpDefinition;

using NodePtr = std::shared_ptr<const Node>;

using NumberPtr = std::shared_ptr<const NumberNode>;
using RationalPtr = std::shared_ptr<const RationalNode>;

using SymbolPtr = std::shared_ptr<const Symbol>;
using VariablePtr = std::shared_ptr<const VariableNode>;

using OpPtr = std::shared_ptr<const OperatorNode>;

using WeakNodePtr = std::weak_ptr<const Node>;


enum class NodeType
{
    Number,
    Variable,
    Operator
};

struct Environment
{
    std::map<std::string,double> map;

    explicit Environment(const std::vector<std::string>& tokens);
    Environment(){};

    double& operator[](const std::string& str) {return map[str];}
    double operator[](const std::string& str) const
    {
        auto it = map.find(str);
        return it!=map.end()? it->second: 0;    
    }

    const double* lookup(const std::string& str) const
    {
        auto it = map.find(str);
        return it!=map.end()? &(it->second): nullptr;    
    }

    double* lookup(const std::string& str)
    {
        auto it = map.find(str);
        return it!=map.end()? &(it->second): nullptr;    
    }

    void print() const
    {
        for(const auto& [k,v]: map)
            std::cout << k << " = " << v << std::endl;
    }
};

class Node : public std::enable_shared_from_this<Node>
{
public:
    
    struct Polynomial
    {
        NodePtr node;
        NumberPtr degree;
        NumberPtr coeff;
    };

    struct ConstPolynomial
    {
        NodePtr node;
        NumberPtr degree;
        NumberPtr coeff;
    };

    virtual ~Node() = default;

    virtual const std::string toString() const = 0;
    virtual const std::string getPostFix() const = 0;
    virtual const std::string getPreFix() const = 0;
    virtual NodeType type() const = 0;

    // virtual const std::string& getIdentifier() const = 0;

    virtual Symbol* asSymbol(){return nullptr;}
    virtual const Symbol* asSymbol() const {return nullptr;}
    virtual NumberNode* asNumber(){return nullptr;}
    virtual const NumberNode* asNumber() const {return nullptr;}

    virtual bool identical(const NodePtr other) const = 0;

    static ConstPolynomial extractMetadata(NodePtr node);

    static int nodeRank(NodePtr node);
    static int compare(NodePtr a,NodePtr b);
    static bool canonicalLess(NodePtr a,NodePtr b);
    static bool canonicalMore(NodePtr a,NodePtr b);

};


static std::string toString(double value);

class NumberNode: public Node
{

public:

    virtual NumberNode* asNumber() override{return this;}
    virtual const NumberNode* asNumber() const override{return this;}

    virtual RationalNode* asRational() {return nullptr;}
    virtual const RationalNode* asRational() const {return nullptr;}

    virtual DecimalNode* asDecimal() {return nullptr;}
    virtual const DecimalNode* asDecimal() const {return nullptr;}
    
    virtual NodeType type() const override {return NodeType::Number;}
    
    virtual const std::string toString() const = 0;
    virtual double getValue()const = 0;
    virtual const std::string getPostFix() const override {return toString();}
    virtual const std::string getPreFix() const override {return toString();}    

    virtual explicit operator double() const {return getValue();};
};
class RationalNode: public NumberNode
{
    long long numerator;
    long long denominator;
public:
    static long long constexpr MAX_DENOMINATOR = 1e12;

    RationalNode():numerator(1),denominator(1){}
    explicit RationalNode(long long num, long long den)
    {
        reduceRational(num,den);
        numerator = num;
        denominator = den;
    }
    RationalNode(long long n):numerator(n),denominator(1){}
    virtual const std::string toString() const override;
    virtual const std::string getPostFix() const override
    {
        if(denominator != 1.0)
            return "(" + std::to_string(numerator) +", " + std::to_string(denominator) + ", RationalNode)";
        else
            return std::to_string(numerator);
    }
    virtual const std::string getPreFix() const override
    {
        if(denominator != 1.0)
            return "RationalNode(" + std::to_string(numerator) +", " + std::to_string(denominator) + ")";
        else
            return std::to_string(numerator);
    }
    double getValue()const override
    {
        if(denominator == 0)
            throw std::runtime_error("Division by zero error!");
        return (double)numerator/(double)denominator;
    }

    long long getNumerator() const {return numerator;}
    long long getDenominator() const {return denominator;}

    virtual bool identical(const NodePtr other) const override;

    virtual RationalNode* asRational() {return this;}
    virtual const RationalNode* asRational() const {return this;}
    RationalNode operator +(const RationalNode& other)
    {
        return RationalNode(numerator*other.denominator + other.numerator*denominator,denominator*other.denominator);
    }
    RationalNode operator -(const RationalNode& other)
    {
        return *this + RationalNode(-other.numerator,other.denominator);
    }
    RationalNode operator *(const RationalNode& other)
    {
        return RationalNode(numerator*other.numerator,denominator*other.denominator);
    }
    RationalNode operator /(const RationalNode& other)
    {
        return RationalNode(numerator*other.denominator,denominator*other.numerator);
    }

    static void reduceRational(long long& numerator, long long& denominator) {
        if (denominator == 0) {
            std::cerr << "Error: Division by zero\n";
            return;
        }

        unsigned long long u_num = (numerator < 0) ? -static_cast<unsigned long long>(numerator) : static_cast<unsigned long long>(numerator);
        unsigned long long u_den = (denominator < 0) ? -static_cast<unsigned long long>(denominator) : static_cast<unsigned long long>(denominator);

        long long gcdVal = static_cast<long long>(std::gcd(u_num, u_den));

        numerator /= gcdVal;
        denominator /= gcdVal;

        if (denominator < 0) {
            numerator = -numerator;
            denominator = -denominator;
        }
    }
};

class Symbol : public Node
{
protected:
    std::string name;
    
public:
    virtual Symbol* asSymbol() override {return this;}
    virtual const Symbol* asSymbol() const override {return this;}

    Symbol():name("NONE"){}
    explicit Symbol(const std::string& name):name(name){}
    explicit Symbol(const Symbol& other):name(other.name){}

    const std::string& getName() const{return name;}

    void setName(const std::string& name){this->name = name;}

    virtual const std::string toString() const = 0;
    virtual const std::string getPostFix() const override {return toString();}
    virtual const std::string getPreFix() const override {return toString();}

    virtual bool identical(const NodePtr other) const override{
        const Symbol* otherSymbol = other->asSymbol();
        if(!otherSymbol)
            return false;
        return name == otherSymbol->name;
    }

    virtual VariableNode* asVariable(){return nullptr;}
    virtual const VariableNode* asVariable()const{return nullptr;}

    virtual OperatorNode* asOperator(){return nullptr;}
    virtual const OperatorNode* asOperator()const{return nullptr;}

    virtual NumberPtr evaluate(const Environment& env) const = 0;

};

// proper variable 
class VariableNode: public Symbol
{
public:
    VariableNode():Symbol(){}
    explicit VariableNode(const std::string&name):Symbol(name){}
    VariableNode(const VariableNode& other):Symbol(other){}
    
    virtual const std::string toString() const override {return name;}
    
    virtual NumberPtr evaluate(const Environment& env)const override;

    virtual VariableNode* asVariable() override {return this;}
    virtual const VariableNode* asVariable()const override{return this;}

    virtual NodeType type() const override {return NodeType::Variable;}


};


class OperatorNode : public Symbol
{
private:
    std::shared_ptr<const OpDefinition> op;
    std::vector<NodePtr> arguments;

public:
    OperatorNode(const std::vector<NodePtr>& args,const std::shared_ptr<const OpDefinition>& op);
    OperatorNode(const OperatorNode& other);

    const std::vector<NodePtr>& getArguments() const {return arguments;}
    NodePtr getArgument(size_t index)
    {
        if(index >= arguments.size() || index < 0)
            throw std::runtime_error("Argument index out of bounds error!");
        return arguments[index];
    }

    const NodePtr getArgument(size_t index) const
    {
        if(index >= arguments.size() || index < 0)
            throw std::runtime_error("Argument index out of bounds error!");
        return arguments[index];
    }

    NodePtr remakeWithArgsFromInterval(size_t start, size_t end) const;
    NodePtr remakeWithoutArgument(size_t element) const;
    std::vector<NodePtr>& getArguments(){return arguments;}

    const std::shared_ptr<const OpDefinition>& getDefinition() const {return op;}
    void acquireArguments(const std::vector<NodePtr>& args)
    {
        this->arguments = std::move(args);
    }
    void setArguments(const std::vector<NodePtr>& args)
    {
        this->arguments = args;
    }
    //canonicallize the operator's arguments for easy comparisons
   
    

    virtual NumberPtr evaluate(const Environment& env) const override;
    virtual const std::string toString() const override;

    virtual OperatorNode* asOperator() override{return this;}
    virtual const OperatorNode* asOperator()const override{return this;}
    
    // sameOperator(OperatorNode a,OperatorNode b) if and only if its name is identical
    virtual bool sameOperator(const NodePtr other) const 
    {return Symbol::identical(other);}
    // identical(OperatorNode a,OperatorNode b) if and only if sameOperator(a,b) and all arguments are identical
    virtual bool identical(const NodePtr other) const override;
    
    virtual NodeType type() const override {return NodeType::Operator;}

    virtual const std::string getPostFix() const override 
    {

          
        std::string str = "(";
        if(arguments.size() < 1)
        {
            return str + name +")";
        }
        str+= arguments[0]->getPostFix();
        for(size_t i=1;i<arguments.size();i++)
        {
            str+= ", " +arguments[i]->getPostFix();
        }
        str+= ", "+ name +")";
        return str;
    }

    virtual const std::string getPreFix() const override 
    {

          
        std::string str = name+"(";
        if(arguments.size() < 1)
        {
            return str + ")";
        }
        str+= arguments[0]->getPreFix();
        for(size_t i=1;i<arguments.size();i++)
        {
            str+= ", " +arguments[i]->getPreFix();
        }
        str+= ")";
        return str;
    }
   
    static OpPtr unify(OpPtr opToSimplify);
    static NodePtr simplify(OpPtr opToSimplify);
    static NodePtr localSimplify(OpPtr opToSimplify);
    static NodePtr expand(OpPtr opToExpand);
    static NodePtr canonicalize(OpPtr opToSimplify);
};


static std::string toString(double value)
{
    std::ostringstream out;
    out << std::setprecision(15) << std::fixed << value;
    std::string str = out.str();
    
    // clean
    str.erase(str.find_last_not_of('0') + 1, std::string::npos);
    if (str.back() == '.') {
        str.pop_back();
    }
    return str;
}
class Casting
{
public:
    static SymbolPtr toSymbol(const NodePtr node);
    static NumberPtr toNumber(const NodePtr node);

    static RationalPtr toRational(const NodePtr node);
    static RationalPtr toRational(const NumberPtr node);

    static VariablePtr toVariable(const SymbolPtr node);
    static VariablePtr toVariable(const NodePtr node);
    static OpPtr toOperator(const SymbolPtr node);
    static OpPtr toOperator(const NodePtr node);
};


};