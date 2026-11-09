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


class Symbol;
class VariableNode;

class SymbolicConstant;

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
    Scalar,
    Variable,
    RawSymbol,
    Operator
};

struct Environment
{
    std::map<NodePtr, NodePtr> map; 

    // precondition: strings must be split such that:
    // the expression to substitute is on the left, and the value it will be substituted for is on the right
    // example: {"x","5","y","5x^2"} would yield a mapping of x=5 and y = 5x^2.
    explicit Environment(const std::vector<std::string>& tokens);
    explicit Environment(const std::map<NodePtr, NodePtr>& mapping);

    Environment(){};

    NodePtr operator[](const NodePtr& node) const {return lookup(node); }
    NodePtr operator[](const std::string& str) const {return lookup(str); }

    NodePtr lookup(const NodePtr& node) const
    {
        auto it = map.find(node);
        return it != map.end() ? it->second : nullptr;    
    }
    NodePtr lookup(const std::string& str) const;

    void setMap(const std::map<NodePtr,NodePtr>& mapping);

};
// stateless
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

    // helper method to check if a node is a child of another node
    // only relevant for certain types of nodes like OperatorNodes, this is a hook
    // all nodes "own" themselves by default
    virtual bool owns(const NodePtr other) const {return this == other.get();}
    // By default, only raw variables are substituteable
    virtual bool isSubstitutable() const {return type() == NodeType::Variable;}
    static ConstPolynomial extractMetadata(NodePtr node);

    static int nodeRank(NodePtr node);
    static int compare(NodePtr a,NodePtr b);
    static bool canonicalLess(NodePtr a,NodePtr b);
    static bool canonicalMore(NodePtr a,NodePtr b);

};


static std::string toString(InfFloat value);
// stateless
class NumberNode: public virtual Node
{

public:
    ~NumberNode() = default;
    virtual NumberNode* asNumber() override{return this;}
    virtual const NumberNode* asNumber() const override{return this;}

    virtual RationalNode* asRational() {return nullptr;}
    virtual const RationalNode* asRational() const {return nullptr;}

    virtual SymbolicConstant* asSymbolicConstant(){return nullptr;}
    virtual const SymbolicConstant* asSymbolicConstant() const {return nullptr;}
    
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
    ~RationalNode() = default;

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

    const auto& getNumerator() const {return mp::numerator(fraction);}
    const auto& getDenominator() const {return mp::denominator(fraction);}

    const InfRational& getFraction() const {return fraction;}

    virtual InfFloat getValue() const { return fraction.convert_to<InfFloat>();}

    virtual RationalNode* asRational() {return this;}
    virtual const RationalNode* asRational() const {return this;}
};


class Symbol: public virtual Node
{
protected:
    std::string name;
    
public:
    virtual Symbol* asSymbol()override {return this;}
    virtual const Symbol* asSymbol() const override {return this;}

    Symbol():name("NONE"){}
    explicit Symbol(const std::string& name):name(name){}
    explicit Symbol(const Symbol& other):name(other.name){}
    ~Symbol() = default;
    const std::string& 
    getName() const{return name;}

    void setName(const std::string& name){this->name = name;}

    virtual const std::string toString() const = 0;
    virtual const std::string getPostFix() const override {return toString();}
    virtual const std::string getPreFix() const override {return toString();}

    virtual VariableNode* asVariable(){return nullptr;}
    virtual const VariableNode* asVariable()const{return nullptr;}

    virtual OperatorNode* asOperator(){return nullptr;}
    virtual const OperatorNode* asOperator()const{return nullptr;}

};

// proper variable, stateless 
class VariableNode: public Symbol
{
public:
    VariableNode():Symbol(){}
    explicit VariableNode(const std::string&name):Symbol(name){}
    VariableNode(const VariableNode& other):Symbol(other){}
    
    virtual const std::string toString() const override {return name;}
    
    virtual VariableNode* asVariable() override {return this;}
    virtual const VariableNode* asVariable()const override{return this;}

    virtual NodeType type() const override {return NodeType::Variable;}


};

// useless symbols that can't actually be substituted for, like the imaginary identity, i
class DummySymbol: public VariableNode
{
public:
    DummySymbol():VariableNode(){}
    explicit DummySymbol(const std::string&name):VariableNode(name){}
    DummySymbol(const DummySymbol& other):VariableNode(other){}

    virtual NodeType type() const override {return NodeType::RawSymbol;}
    virtual bool isSubstitutable() const override{ return false;}
};


class SymbolicConstant : public Symbol, public NumberNode
{
    InfFloat val;
public:
    ~SymbolicConstant() = default;
    SymbolicConstant():val(0){}
    explicit SymbolicConstant(const std::string& name,const InfFloat& val):Symbol(name),val(val){}

    InfFloat getValue()const override{return val;}

    const InfFloat& getFloat() const {return val;}

    virtual const std::string toString() const override {return name;}
    virtual const std::string getPostFix() const override {return "("+name+",SymbolicConstant)";}
    virtual const std::string getPreFix() const override {return "SymbolicConstant("+name+")";}   
    virtual NodeType type() const override {return NodeType::Scalar;}

    virtual SymbolicConstant* asSymbolicConstant() override {return this;}
    virtual const SymbolicConstant* asSymbolicConstant() const override {return this;}


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

    virtual const std::string toString() const override;

    virtual OperatorNode* asOperator() override{return this;}
    virtual const OperatorNode* asOperator()const override{return this;}
    
    // sameOperator(OperatorNode a,OperatorNode b) if and only if its name is identical
    virtual bool sameOperator(const NodePtr other) const;
    
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

    virtual bool owns(const NodePtr other) const override;
    virtual bool isSubstitutable() const override;

    static OpPtr unify(OpPtr opToSimplify);
    static NodePtr simplify(OpPtr opToSimplify);
    static NodePtr localSimplify(OpPtr opToSimplify);
    static NodePtr expand(OpPtr opToExpand);
    static NodePtr canonicalize(OpPtr opToSimplify);
    static NodePtr factorize(OpPtr opToFactorize);

    // Preconditions: opToApproximate must not contain any unsubstituted variables
    // It can only accept an operator with constant approximatables inside.
    // Will substitute 0 if it encounters a variable node.
    static InfFloat approximate(OpPtr opToApproximate);
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
     // purposefully fails if the node is a SymbolicConstant
    static NumberPtr toRawNumber(const NodePtr node);
    static NumberPtr toNumber(const NodePtr node);

    static RationalPtr toRational(const NodePtr node);
    static RationalPtr toRational(const NumberPtr node);

    static VariablePtr toVariable(const SymbolPtr node);
    static VariablePtr toVariable(const NodePtr node);
    static OpPtr toOperator(const SymbolPtr node);
    static OpPtr toOperator(const NodePtr node);
};


};