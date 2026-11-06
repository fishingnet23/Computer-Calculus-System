#pragma once
#include "../include/Config.hpp"
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
class FloatNode;

class Symbol;
class VariableNode;

class OperatorNode;
class OpDefinition;

using NodePtr = std::shared_ptr<const Node>;

using NumberPtr = std::shared_ptr<const NumberNode>;
using RationalPtr = std::shared_ptr<const RationalNode>;
using FloatPtr = std::shared_ptr<const FloatNode>;

using SymbolPtr = std::shared_ptr<const Symbol>;
using VariablePtr = std::shared_ptr<const VariableNode>;

using OpPtr = std::shared_ptr<const OperatorNode>;

using WeakNodePtr = std::weak_ptr<const Node>;


enum class NodeType
{
    Scalar,
    Variable,
    Operator
};

struct Environment
{
    std::map<std::string, InfFloat> map; 

    explicit Environment(const std::vector<std::string>& tokens);
    Environment(){};

    InfFloat& operator[](const std::string& str) { return map[str]; }
    
    InfFloat operator[](const std::string& str) const
    {
        auto it = map.find(str);
        return it != map.end() ? it->second : InfFloat(0);    
    }

    const InfFloat* lookup(const std::string& str) const
    {
        auto it = map.find(str);
        return it != map.end() ? &(it->second) : nullptr;    
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


static std::string toString(InfFloat value);

class NumberNode: public Node
{

public:

    virtual NumberNode* asNumber() override{return this;}
    virtual const NumberNode* asNumber() const override{return this;}

    virtual RationalNode* asRational() {return nullptr;}
    virtual const RationalNode* asRational() const {return nullptr;}

    virtual FloatNode* asFloat() {return nullptr;}
    virtual const FloatNode* asFloat() const {return nullptr;}
    
    virtual NodeType type() const override {return NodeType::Scalar;}
    
    virtual const std::string toString() const = 0;
    virtual InfFloat getValue()const = 0;

    virtual int compare(const NumberPtr& other) const {
        auto val1 = getValue();
        auto val2 = other->getValue();
        
        if (val1 < val2) return -1;
        if (val1 > val2) return 1;
        return 0;
    }


    virtual const std::string getPostFix() const override {return toString();}
    virtual const std::string getPreFix() const override {return toString();}    

    virtual explicit operator InfFloat() const {return getValue();};
};
class RationalNode: public NumberNode
{
    InfRational fraction;

    // display helper
    std::string compressLargeInteger(const std::string& raw_str) const {
        if (raw_str.length() > 12) {
            std::stringstream compressed;
            compressed << raw_str[0] << "." << raw_str.substr(1, 4);
            
            std::string sig_figs = compressed.str();
            sig_figs.erase(sig_figs.find_last_not_of('0') + 1, std::string::npos);
            if (sig_figs.back() == '.') {
                sig_figs.pop_back();
            }
            
            size_t exponent = raw_str.length() - 1;
            return sig_figs + "...e+" + std::to_string(exponent);
        }
        return raw_str;
    }
public:
    RationalNode():fraction(0,1){}
    explicit RationalNode(long long num, long long den):fraction(num,den){}
    explicit RationalNode(const InfInt& num, const InfInt& den):fraction(num,den){}
    RationalNode(long long n):fraction(n,1){}
    RationalNode(const InfInt& n):fraction(n,1){}
    RationalNode(const InfFloat& v):fraction(v.convert_to<InfRational>()){}
    RationalNode(const InfRational& r):fraction(r){}
    virtual const std::string toString() const override {
        const auto& numerator = mp::numerator(fraction);
        const auto& denominator = mp::denominator(fraction);

        if (denominator == 1) {
            return compressLargeInteger(numerator.str());
        }
        return compressLargeInteger(numerator.str()) + "/" + compressLargeInteger(denominator.str());
    }

    virtual const std::string getPostFix() const override
    {
        const auto& numerator = mp::numerator(fraction);
        const auto& denominator = mp::denominator(fraction);
        
        std::string num_str = compressLargeInteger(numerator.str());
        std::string den_str = compressLargeInteger(denominator.str());

        if (denominator != 1)
            return "(" + num_str + ", " + den_str + ", RationalNode)";
        else
            return num_str;
    }

    virtual const std::string getPreFix() const override
    {
        const auto& numerator = mp::numerator(fraction);
        const auto& denominator = mp::denominator(fraction);

        std::string num_str = compressLargeInteger(numerator.str());
        std::string den_str = compressLargeInteger(denominator.str());

        if (denominator != 1)
            return "RationalNode(" + num_str + ", " + den_str + ")";
        else
            return num_str;
    }

    const InfInt& getNumerator() const {return mp::numerator(fraction);}
    const InfInt& getDenominator() const {return mp::denominator(fraction);}

    const InfRational& getFraction() const {return fraction;}

    virtual bool identical(const NodePtr other) const override;

    virtual InfFloat getValue() const { return fraction.convert_to<InfFloat>();}

    virtual RationalNode* asRational() {return this;}
    virtual const RationalNode* asRational() const {return this;}
};

class FloatNode: public NumberNode
{
    InfFloat val;
public:

    FloatNode():val(0){}
    explicit FloatNode(long long num):val(num){}
    FloatNode(const InfRational& n) {
        InfFloat num(mp::numerator(n).str());
        InfFloat den(mp::denominator(n).str());
        val = num / den;
    }
    explicit FloatNode(const InfInt& num, const InfInt& den) {
        InfFloat n(num.str());
        InfFloat d(den.str());
        val = n / d;
    }
    FloatNode(const InfInt& n):val(n.str()){}
    FloatNode(const InfFloat& n):val(n){}

    virtual const std::string toString() const override;
    virtual const std::string getPostFix() const override{return val.str();}
    virtual const std::string getPreFix() const override{return val.str();}
    InfFloat getValue()const override{return val;}

    const InfFloat& getFloat() const {return val;}

    virtual bool identical(const NodePtr other) const override;

    virtual FloatNode* asFloat() {return this;}
    virtual const FloatNode* asFloat() const {return this;}
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


static std::string toString(InfFloat value)
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
class SoftCasting
{
public:
    static SymbolPtr toSymbol(const NodePtr node);
    static NumberPtr toNumber(const NodePtr node);

    static RationalPtr toRational(const NodePtr node);
    static RationalPtr toRational(const NumberPtr node);

    static FloatPtr toFloat(const NodePtr node);
    static FloatPtr toFloat(const NumberPtr node);

    static VariablePtr toVariable(const SymbolPtr node);
    static VariablePtr toVariable(const NodePtr node);
    static OpPtr toOperator(const SymbolPtr node);
    static OpPtr toOperator(const NodePtr node);
};


};