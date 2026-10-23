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
namespace AST
{

class Node;
class Symbol;
class NumberNode;
class VariableNode;
class OperatorNode;
class OpDefinition;

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

    double& operator[](const std::string& str){return map[str];}
    double operator[](const std::string& str) const
    {
        auto it = map.find(str);
        return it!=map.end()? it->second: 0;    
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
    
    virtual ~Node() = default;

    virtual double evaluate(const Environment& env) const = 0;
    virtual const std::string toString() const = 0;
    virtual const std::string getPostFix() const = 0;
    virtual NodeType type() const = 0;
    virtual std::shared_ptr<Node> clone() const = 0; 

    virtual Symbol* asSymbol(){return nullptr;}
    virtual const Symbol* asSymbol() const {return nullptr;}
    virtual NumberNode* asNumber(){return nullptr;}
    virtual const NumberNode* asNumber() const {return nullptr;}

    virtual bool identical(const Node* other) const = 0;
    virtual bool identical(const std::shared_ptr<Node>& other){return this->identical(other.get());};


    static int nodeRank(const std::shared_ptr<Node>& node);
    static bool canonicalLess(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);

};
using NodePtr = std::shared_ptr<Node>;

static std::string toString(double value);

class NumberNode: public Node
{
    double value;
public:
    explicit NumberNode(double val){value = val; }

    explicit NumberNode(const std::string& str)
    {
        try
        {
            value = std::stod(str);
        }
        catch(const std::exception& e)
        {
            std::cerr << "Error when trying to create a number node from a string: " << e.what()<<", defaulting to 0." << '\n';
            value = 0;
        }
        
    }

    virtual NumberNode* asNumber() override{return this;}
    virtual const NumberNode* asNumber() const override{return this;}
    virtual NodeType type() const override {return NodeType::Number;}
    
    double getValue()const{return value;}
    void setValue(double value){this->value=value;}

    virtual double evaluate(const Environment& env) const override{return value;};
    virtual const std::string toString() const override
    {
       return AST::toString(value);
    }
    virtual const std::string getPostFix() const override {return toString();}
    virtual bool identical(const Node* other) const override {return other->asNumber() && other->asNumber()->value == value;}
    
    virtual std::shared_ptr<Node> clone() const override {return std::make_shared<NumberNode>(value);} 
 

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
    virtual bool identical(const Node* other) const override{
        const Symbol* otherSymbol = other->asSymbol();
        if(!otherSymbol)
            return false;
        return name == otherSymbol->name;
    }

    virtual std::shared_ptr<Node> simplify();

    virtual VariableNode* asVariable(){return nullptr;}
    virtual const VariableNode* asVariable()const{return nullptr;}

    virtual OperatorNode* asOperator(){return nullptr;}
    virtual const OperatorNode* asOperator()const{return nullptr;}

};

class VariableNode: public Symbol
{
    // atomic variables store doubles for coefficients and degrees
    double coeff;
    double degree; 
public:
    VariableNode():coeff(1),degree(1),Symbol(){}
    VariableNode(const std::string&name, double coeff, double degree):coeff(coeff),degree(degree),Symbol(name){}
    VariableNode(const VariableNode& other):coeff(other.coeff),degree(other.degree),Symbol(other){}
    
    virtual const std::string toString() const override {return name;}
    
    virtual double evaluate(const Environment& env)const override;

    virtual VariableNode* asVariable() override {return this;}
    virtual const VariableNode* asVariable()const override{return this;}

    virtual NodeType type() const override {return NodeType::Variable;}

    virtual std::shared_ptr<Node> clone() const override {return std::make_shared<VariableNode>(*this);} 

    void setCoeff(double coeff){this->coeff=coeff;}
    void setDegree(double degree){this->degree = degree;}

};

class OperatorNode : public Symbol
{
private:
    std::shared_ptr<const OpDefinition> op;
    std::vector<NodePtr> arguments;

public:
    OperatorNode(const std::vector<NodePtr>& args,const std::shared_ptr<const OpDefinition>& op);
    OperatorNode(const std::vector<NodePtr>& args,const std::shared_ptr<const OpDefinition>& op, double coeff, double degree);
    OperatorNode(const OperatorNode& other);

    const std::vector<NodePtr>& getArguments() const {return arguments;}
    std::shared_ptr<Node> remakeWithArgsFromInterval(size_t start, size_t end) const;
    std::shared_ptr<Node> remakeWithoutArgument(size_t element) const;
    std::vector<NodePtr>& getArguments(){return arguments;}

    std::shared_ptr<const OpDefinition>& getDefinition(){return op;}
    void acquireArguments(const std::vector<NodePtr>& args)
    {
        this->arguments = std::move(args);
    }
    void setArguments(const std::vector<NodePtr>& args)
    {
        this->arguments = args;
    }
    //canonicallize the operator's arguments for easy comparisons
    void unify();

    virtual std::shared_ptr<Node> simplify() override;
    virtual std::shared_ptr<Node> expand();

    std::shared_ptr<Node> canonicalize();

    virtual double evaluate(const Environment& env) const override;
    virtual const std::string toString() const override;

    virtual OperatorNode* asOperator() override{return this;}
    virtual const OperatorNode* asOperator()const override{return this;}
    
    // sameOperator(OperatorNode a,OperatorNode b) if and only if its name, coeffecient and degree are identical
    virtual bool sameOperator(const Node* other) const 
    {return other->asSymbol() != nullptr && other->asSymbol()->asOperator() != nullptr && other->asSymbol()->Symbol::identical(this);}
    // identical(OperatorNode a,OperatorNode b) if and only if sameOperator(a,b) and all arguments are identical
    virtual bool identical(const Node* other) const override;
    
    virtual NodeType type() const override {return NodeType::Operator;}
    
    virtual std::shared_ptr<Node> clone() const override
    {
        return std::make_shared<OperatorNode>(*this);
    }

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
};

using SymbolPtr = std::shared_ptr<Symbol>;
using NumberPtr = std::shared_ptr<NumberNode>;
using VariablePtr = std::shared_ptr<VariableNode>;
using OpPtr = std::shared_ptr<OperatorNode>;

static inline SymbolPtr toSymbol(const NodePtr& node)
{
    if (!node) return nullptr;
    return node->asSymbol()? std::static_pointer_cast<Symbol>(node):nullptr;
}
static inline NumberPtr toNumber(const NodePtr& node)
{
    if (!node) return nullptr;
    return node->asNumber()? std::static_pointer_cast<NumberNode>(node):nullptr;
}

static inline VariablePtr toVariable(const SymbolPtr& node)
{
    if (!node) return nullptr;
    return node->asVariable()? std::static_pointer_cast<VariableNode>(node):nullptr;
}
static inline VariablePtr toVariable(const NodePtr& node)
{
    if (!node) return nullptr;
    auto symbol = node->asSymbol();
    return (symbol && symbol->asVariable()) 
        ? std::static_pointer_cast<VariableNode>(node) 
        : nullptr;
}
static inline OpPtr toOperator(const SymbolPtr& node)
{
    if (!node) return nullptr;
    return node->asOperator()? std::static_pointer_cast<OperatorNode>(node):nullptr;
}
static inline OpPtr toOperator(const NodePtr& node)
{
    if (!node) return nullptr;
    return node->asSymbol() && node->asSymbol()->asOperator() 
        ? std::static_pointer_cast<OperatorNode>(node) 
        : nullptr;
}

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


};