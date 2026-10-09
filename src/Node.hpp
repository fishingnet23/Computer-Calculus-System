#pragma once


#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <map>

namespace AST
{
class Environment
{
    std::unordered_map<std::string, double> data;
public:
    Environment(){}
    explicit Environment(const std::vector<std::string>& tokens);
    double& operator[](const std::string& token) {return data[token];}
    auto find(const std::string& token) const {return data.find(token);}
    auto end() const {return data.end();}
    auto begin() const {return data.begin();}
    void print()
    {
        std::map<std::string, double> sorted_env(data.begin(), data.end());
        for (const auto& pair : sorted_env)
        {
            std::cout << pair.first << ": " << pair.second << '\n';
        }
    }

    static bool isCommonIdentifier(const std::string& src)
    {
        return src == "x" || src == "y" || src == "z";
    }
};
enum class OptimizationPhase {
    EXPAND,
    COMPRESS
};

class Symbol;

enum class NodeType
{
    Unknown = 0,
    Number = 1,
    Variable = 2,
    Operator = 3,
};

class Node {
public:
    NodeType type;
    virtual ~Node() = default;
    Node(const Node& other):type(other.type){}
    Node(){type = NodeType::Unknown;}
    Node(const NodeType& type){this->type=type;}

    // pure virtual functions
    
    virtual std::string toString() const = 0;
    virtual double evaluate(const Environment& env) const = 0;
    // strict equality
    bool virtual equals(const Node* other) const = 0;
    virtual std::string getName() const = 0;
    
    virtual std::shared_ptr<Node> clone() const = 0;

    // upcast helpers
    virtual Symbol* asSymbol() {return nullptr;}
    virtual const Symbol* asSymbol() const {return nullptr;}

    static int nodeRank(const std::shared_ptr<Node>& node);
    static bool canonicalLess(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);
};

class Symbol : public Node
{
static const int MAX_SIMPLIFICATION_STEPS = 50; // Maximum number of simplification steps to avoid infinite loops

protected:
    double coeffecient;
    double degree;

public:
    Symbol():degree(1),coeffecient(1){}
    Symbol(double coeff, double degree):coeffecient(coeff),degree(degree){}
    Symbol(const Symbol& other):coeffecient(other.coeffecient),degree(other.degree){}
    // upcast helpers from Node baseclass
    virtual Symbol* asSymbol() override {return this;};
    virtual const Symbol* asSymbol() const override {return this;};

    void setCoeffecient(double coeffecient){this->coeffecient = coeffecient;}
    void setDegree(double degree){this->degree = degree;}
    virtual void setName(const std::string& name) = 0;

    virtual double getDegree() const {return degree;}
    virtual double getCoeffecient() const {return coeffecient;}

    void negate(){coeffecient = -coeffecient;}
    double negate()const{return -coeffecient;}

    void inverse()
    {
        if(coeffecient == 0)
            throw std::runtime_error("Division by zero error!");

        coeffecient = 1.0 / coeffecient;
        degree = -degree;
    }


    virtual bool equalBases(const Symbol* other) const = 0;
    virtual std::shared_ptr<Node> simplifyStep(OptimizationPhase phase) const = 0;
    
    std::shared_ptr<Node> simplify() const;
    
    static std::shared_ptr<Node> applySymbolData(const std::shared_ptr<Node>& node,double coefficient,double degree);
};

using NodePtr = std::shared_ptr<Node>;




};