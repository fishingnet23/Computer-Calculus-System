#pragma once
#include "Node.hpp"
#include "Operators.hpp"

// simple wrapper for a double value
class NumberNode : public Node {
    double value;
public:
    NumberNode(double val) : value(val) {}
    std::string toString() const override { return std::to_string(value); }
    double evaluate(const Environment&) const override { return value; }
};

class VariableNode : public Node {
    std::string name;
public:
    VariableNode(std::string name) : name(std::move(name)) {}
    std::string toString() const override { return name; }

    //method that simply looks up the variables' value
    double evaluate(const Environment& env) const override {
        // look up the variable name in the enviornment table. If it's not there, default to 0.
        auto pair = env.find(name);
        return (pair != env.end()) ? pair->second : 0.0;
    }
};

class OperatorNode : public Node {
    std::string op;
    std::vector<NodePtr> arguments;
    const OperationRegistry& procedureRegistry;

public:
    OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry& registry)
        : op(std::move(op)), arguments(std::move(args)), procedureRegistry(registry) {
    }

    std::string toString() const override {
        // Unary operator: op(arg)
        if (arguments.size() == 1) {
            return op + "(" + arguments[0]->toString() + ")";
        }
        // Binary operator: arg1 op arg2
        if (arguments.size() == 2) {
            return arguments[0]->toString() + " " + op + " " + arguments[1]->toString();
        }
        
        std::string str = "(";
        for (const auto& arg : arguments)
        {
            str += arg->toString() + " ";
        }
        str += ")";
        return str;
    }

    double evaluate(const Environment& env) const {
        // Evaluate all child nodes first, this ensures nested operations and functions get evaluated first
        std::vector<double> evaluatedArgs;
        for (const auto& child : arguments) {
            evaluatedArgs.push_back(child->evaluate(env));
        }

        // Look up in the registry and calculate the final result
        return procedureRegistry.execute(op, evaluatedArgs);
    }
};



