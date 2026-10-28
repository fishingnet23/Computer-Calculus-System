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
class PlaceholderNode;
class OperatorNode;
class OpDefinition;

using NodePtr = std::shared_ptr<Node>;
using ConstNodePtr = std::shared_ptr<const Node>;
using SymbolPtr = std::shared_ptr<Symbol>;
using NumberPtr = std::shared_ptr<NumberNode>;
using VariablePtr = std::shared_ptr<VariableNode>;
using PlaceholderPtr = std::shared_ptr<PlaceholderNode>;

using OpPtr = std::shared_ptr<OperatorNode>;

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
        double degree;
        double coeff;
    };

    struct ConstPolynomial
    {
        ConstNodePtr node;
        double degree;
        double coeff;
    };

    virtual ~Node() = default;

    virtual double evaluate(const Environment& env) const = 0;
    virtual const std::string toString() const = 0;
    virtual const std::string getPostFix() const = 0;
    virtual NodeType type() const = 0;
    virtual NodePtr clone() const = 0; 

    // virtual const std::string& getIdentifier() const = 0;

    virtual Symbol* asSymbol(){return nullptr;}
    virtual const Symbol* asSymbol() const {return nullptr;}
    virtual NumberNode* asNumber(){return nullptr;}
    virtual const NumberNode* asNumber() const {return nullptr;}
    virtual PlaceholderNode* asPlaceholder(){return nullptr;}
    virtual const PlaceholderNode* asPlaceholder()const{return nullptr;}

    virtual bool identical(const ConstNodePtr other) const = 0;

    static ConstPolynomial extractMetadata(ConstNodePtr node);

    static int nodeRank(ConstNodePtr node);
    static int compare(ConstNodePtr a,ConstNodePtr b);
    static bool canonicalLess(ConstNodePtr a,ConstNodePtr b);
    static bool canonicalMore(ConstNodePtr a,ConstNodePtr b);



};


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
    virtual bool identical(const ConstNodePtr other) const override {return other->asNumber() && other->asNumber()->value == value;}
    
    virtual NodePtr clone() const override {return std::make_shared<NumberNode>(value);} 
 

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
    virtual bool identical(const ConstNodePtr other) const override{
        const Symbol* otherSymbol = other->asSymbol();
        if(!otherSymbol)
            return false;
        return name == otherSymbol->name;
    }

    virtual NodePtr simplify();

    virtual VariableNode* asVariable(){return nullptr;}
    virtual const VariableNode* asVariable()const{return nullptr;}

    virtual OperatorNode* asOperator(){return nullptr;}
    virtual const OperatorNode* asOperator()const{return nullptr;}

};

// proper variable 
class VariableNode: public Symbol
{
public:
    VariableNode():Symbol(){}
    explicit VariableNode(const std::string&name):Symbol(name){}
    VariableNode(const VariableNode& other):Symbol(other){}
    
    virtual const std::string toString() const override {return name;}
    
    virtual double evaluate(const Environment& env)const override;

    virtual VariableNode* asVariable() override {return this;}
    virtual const VariableNode* asVariable()const override{return this;}

    virtual NodeType type() const override {return NodeType::Variable;}

    virtual NodePtr clone() const override {return std::make_shared<VariableNode>(*this);} 

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
    NodePtr getArgument(size_t index)
    {
        if(index >= arguments.size() || index < 0)
            throw std::runtime_error("Argument index out of bounds error!");
        return arguments[index];
    }

    const ConstNodePtr getArgument(size_t index) const
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
    void unify();

    virtual NodePtr simplify() override;
    virtual NodePtr localSimplify();
    virtual NodePtr expand();

    NodePtr canonicalize();

    virtual double evaluate(const Environment& env) const override;
    virtual const std::string toString() const override;

    virtual OperatorNode* asOperator() override{return this;}
    virtual const OperatorNode* asOperator()const override{return this;}
    
    // sameOperator(OperatorNode a,OperatorNode b) if and only if its name is identical
    virtual bool sameOperator(const ConstNodePtr other) const 
    {return Symbol::identical(other);}
    // identical(OperatorNode a,OperatorNode b) if and only if sameOperator(a,b) and all arguments are identical
    virtual bool identical(const ConstNodePtr other) const override;
    
    virtual NodeType type() const override {return NodeType::Operator;}
    
    virtual NodePtr clone() const override
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
    static SymbolPtr toSymbol(const NodePtr& node);
    static NumberPtr toNumber(const NodePtr& node);
    static VariablePtr toVariable(const SymbolPtr& node);
    static VariablePtr toVariable(const NodePtr& node);
    static OpPtr toOperator(const SymbolPtr& node);
    static OpPtr toOperator(const NodePtr& node);
};


};