#pragma once
#include "Node.hpp"
#include "Operators.hpp"
#include <algorithm>


namespace AST
{
// simple wrapper for a double value
class NumberNode : public Node {
    double value;
public:
    NumberNode(double val) : value(val) {arguments = {}; father = nullptr;}
    NumberNode(const std::string& val)
    {
        try
        {
            value = std::stod(val);
        }
        catch (const std::invalid_argument& e) {
            std::cout << "Error: String could not be converted to a number.\n";
        }
        catch (const std::out_of_range& e) {
            std::cout << "Error: Number is too large for the target type.\n";
        }
        arguments = {}; father = nullptr;
    }
    NumberNode(const NumberNode& other) : value(other.value) {arguments = {}; father = nullptr;}
    NumberNode(double val, Node* parent) : value(val) {arguments = {}; father = parent;}

    std::string toString() const override { return std::to_string(value); }
    // hook to base class
    double evaluate(const Environment&) const override { return value; }
    // this is a number. its value is constant.
    double evaluate() const { return value; }

    NodePtr simplifyStep() override { return std::make_shared<NumberNode>(value); }
    bool equals(const Node* other) const override
    {
        const auto num = dynamic_cast<const NumberNode*>(other);
        if(!num)
            return false;
        return value == num->value;
    }
    std::string getName() const override {return std::to_string(value);}
};

class VariableNode : public Node {
    std::string name;
    double coeffecient = 1.0;
    double degree = 1.0;
public:
    explicit VariableNode(const std::string& name) : name(name),degree(1.0),coeffecient(1.0) {arguments = {}; father = nullptr;}
    VariableNode(const VariableNode& other) : name(other.name),degree(other.degree),coeffecient(other.coeffecient) {arguments = {};   father = nullptr;}
    VariableNode(const std::string& name, Node* parent) : name(name) {arguments = {}; father = parent;}
    VariableNode(const std::string& name, double coeffecient, double degree): name(name),coeffecient(coeffecient),degree(degree){arguments = {}; father = nullptr;}
    VariableNode(const std::string& name, double coeffecient, double degree,Node* parent): name(name),coeffecient(coeffecient),degree(degree){arguments = {}; father = parent;}

    std::string toString() const override
    {
        std::string res = "";

        if(coeffecient != 1.0 && coeffecient != -1.0)
            res += std::to_string(coeffecient);
        if(coeffecient == -1.0)
            res += "-";
        res += name;

        if (degree != 1.0)
            res += "^" + std::to_string(degree);

        return res;
    }
    void negate(){coeffecient = -coeffecient;}


    //method that simply looks up the variables' value
    double evaluate(const Environment& env) const override {
        // look up the variable name in the enviornment table. If it's not there, default to 0.
        auto pair = env.find(name);
        return (pair != env.end()) ? coeffecient * std::pow(pair->second,degree) : 0.0;
    }
    NodePtr simplifyStep() override { 
        if(coeffecient == 0)
            return std::make_shared<NumberNode>(0);
        else if(degree == 0) // accounts for 0^0 by just defaulting to 0
            return std::make_shared<NumberNode>(1);
        return std::make_shared<VariableNode>(*this); }
    bool equals(const Node* other) const override
    {
        const auto var = dynamic_cast<const VariableNode*>(other);

        if (!var)
            return false;

        return name == var->name
            && coeffecient == var->coeffecient
            && degree == var->degree;
    }

    // getters
    std::string getName() const override {return name;}
    double getCoeffecient() const {return coeffecient;}
    double getDegree() const {return degree;}

    // setters
    void setName(const std::string& name){this->name = name;}
    void setCoeffecient(double coeffecient){this->coeffecient = coeffecient;}
    void setDegree(double degree){this->degree = degree;}


};

class OperatorNode : public VariableNode {
    std::string op;
    const OperationRegistry& procedureRegistry;


    // Helper function to flatten arguments of the same operator, useful for associative operations like addition and multiplication
    static std::vector<NodePtr> flattenArgumentsWithSharedOperator(const std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry);
    static std::shared_ptr<OperatorNode> populateTreeFromVectorUsingSharedOperator(std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry);
    static std::vector<std::vector<NodePtr>> groupLikeTerms(const std::vector<NodePtr>& args);
    static NodePtr reduceGroup(const std::vector<NodePtr>& group,const std::string& op,const OperationRegistry& registry);

    bool equals(const OperatorNode* other) const;

public:
    OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry& registry);
    OperatorNode(std::string op, std::vector<NodePtr> args,Node* parent, const OperationRegistry& registry);
    OperatorNode(const OperatorNode& other);

    std::string toString() const override;
    const std::string& getOperand() const {return op;}
    std::string getName() const override {return op;}

    double evaluate(const Environment& env) const override;

    NodePtr simplifyStep() override;    

    bool equals(const Node* other) const override
    {
        const auto op = dynamic_cast<const OperatorNode*>(other);
        if(!op)
            return false;
        return equals(op);
    }
};
    
};
