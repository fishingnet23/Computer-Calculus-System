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
    NumberNode(double val) : value(val){type = NodeType::Number;}
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
    type = NodeType::Number;
    }
    NumberNode(const NumberNode& other) : value(other.value){type = NodeType::Number;}

    std::shared_ptr<Node> clone() const override {
        return std::make_shared<NumberNode>(*this);
    }

    std::string toString() const override { return std::to_string(value); }
    // hook to base class
    double evaluate(const Environment&) const override { return value; }
    // this is a number. its value is constant.
    double evaluate() const { return value; }

    bool equals(const Node* other) const override
    {
        if(other->type!= NodeType::Number)
            return false;
        const auto num = static_cast<const NumberNode*>(other);
        return value == num->value;
    }

    std::string getName() const override {return std::to_string(value);}

};

class VariableNode : public Symbol {
    std::string name;
public:
    explicit VariableNode(const std::string& name) : name(name),Symbol(1.0,1.0){ type = NodeType::Variable; }
    VariableNode(const VariableNode& other) : name(other.name),Symbol(other){ type = NodeType::Variable; }
    VariableNode(const std::string& name, double coeffecient, double degree): name(name),Symbol(coeffecient,degree){ type = NodeType::Variable; }

    std::shared_ptr<Node> clone() const override {
        return std::make_shared<VariableNode>(*this);
    }

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

    //method that simply looks up the variables' value
    double evaluate(const Environment& env) const override {
        // look up the variable name in the enviornment table. If it's not there, default to 0.
        auto pair = env.find(name);
        return (pair != env.end()) ? coeffecient * std::pow(pair->second,degree) : 0.0;
    }
    NodePtr simplifyStep(OptimizationPhase phase) const override { 
        if(coeffecient == 0)
            return std::make_shared<NumberNode>(0);
        else if(degree == 0) // accounts for 0^0 by just defaulting to 0
            return std::make_shared<NumberNode>(coeffecient);
        return std::make_shared<VariableNode>(*this); }
    bool equals(const Node* other) const override
    {
        if(other->type != NodeType::Variable)
            return false;
        const auto var = static_cast<const VariableNode*>(other);

        return name == var->name
            && coeffecient == var->coeffecient
            && degree == var->degree;
    }
    virtual bool equalBases(const Symbol* other) const override
    {
       if(other->type != NodeType::Variable)
            return false;
        const auto var = static_cast<const VariableNode*>(other);

        return name == var->name;
    } 

    // getters
    std::string getName() const override {return name;}

    // setters
    void setName(const std::string& name){this->name = name;}

};

class OperatorNode : public Symbol {
    std::string op;
    std::vector<NodePtr> arguments;
    const OperationRegistry& procedureRegistry;


    // Helper function to flatten arguments of the same operator, useful for associative operations like addition and multiplication
    static std::vector<NodePtr> flattenArgumentsWithSharedOperator(const std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry);
    static NodePtr populateTreeFromVectorUsingSharedOperator(std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry,double coeffecient, double degree);
    static std::vector<std::vector<NodePtr>> groupLikeTerms(const std::vector<NodePtr>& args);
    static NodePtr reduceGroup(const std::vector<NodePtr>& group,const std::string& op,const OperationRegistry& registry);

public:
    OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry& registry);
    OperatorNode(const OperatorNode& other);

    std::shared_ptr<Node> clone() const override {
        return std::make_shared<OperatorNode>(*this);
    }

    std::string toString() const override;
    const std::string& getOperand() const {return op;}
    std::string getName() const override {return op;}
    const OperationRegistry& getRegistry() const{return procedureRegistry;}
    void setName(const std::string& name){op=name;}

    double evaluate(const Environment& env) const override;

    NodePtr simplifyStep(OptimizationPhase phase) const override;    

    bool equals(const Node* other) const override;
    virtual bool equalBases(const Symbol* other) const override;

    const auto& getArguments()const{return arguments;}
    void setArguments(std::vector<NodePtr> arguments){
        this->arguments.clear();
        this->arguments.reserve(arguments.size());

        for (const auto& arg : arguments) {
            this->arguments.push_back(arg->clone());
        }
    }
};
    
};
