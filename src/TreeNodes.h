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
    double evaluate(const Environment&) const override { return value; }
    NodePtr simplifyStep() override { return std::make_shared<NumberNode>(value); }
};

class VariableNode : public Node {
    std::string name;
public:
    VariableNode(const std::string& name) : name(std::move(name)) {arguments = {}; father = nullptr;}
    VariableNode(const VariableNode& other) : name(other.name) {arguments = {}; father = nullptr;}
    VariableNode(const std::string& name, Node* parent) : name(name) {arguments = {}; father = parent;}
    std::string toString() const override { return name; }

    //method that simply looks up the variables' value
    double evaluate(const Environment& env) const override {
        // look up the variable name in the enviornment table. If it's not there, default to 0.
        auto pair = env.find(name);
        return (pair != env.end()) ? pair->second : 0.0;
    }
    NodePtr simplifyStep() override { return std::make_shared<VariableNode>(name); }

};

class OperatorNode : public Node {
    std::string op;
    const OperationRegistry& procedureRegistry;

    // Helper function to flatten arguments of the same operator, useful for associative operations like addition and multiplication
    static std::vector<NodePtr> flattenArgumentsWithSharedOperator(const std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry);
    static std::shared_ptr<OperatorNode> populateTreeFromVectorUsingSharedOperator(std::vector<NodePtr>& args, const std::string& op, const OperationRegistry& registry);

public:
    OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry& registry);
    OperatorNode(std::string op, std::vector<NodePtr> args,Node* parent, const OperationRegistry& registry);

    std::string toString() const override;
    double evaluate(const Environment& env) const override;

    NodePtr simplifyStep() override;
    const std::vector<NodePtr>& getArguments() const { return arguments; }
};
    
};
