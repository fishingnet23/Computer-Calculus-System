#pragma once
#include "Node.hpp"
#include "Operators.hpp"



// simple wrapper for a double value
class NumberNode : public ASTNode {
    double value;
public:
    NumberNode(double val) : value(val) {}
    NumberNode(std::string val)
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
    }
    std::string toString() const override { return std::to_string(value); }
    double evaluate(const Environment&) const override { return value; }
};

class VariableNode : public ASTNode {
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

class OperatorNode : public ASTNode {
    std::string op;
    std::vector<ASTNodePtr> arguments;
    const OperationRegistry& procedureRegistry;

public:
    OperatorNode(std::string op, std::vector<ASTNodePtr> args, const OperationRegistry& registry)
        : op(std::move(op)), arguments(std::move(args)), procedureRegistry(registry) {
    }

    std::string toString() const override {
        // Unary operator: op(arg)
        if (arguments.size() == 1) {
            return op + "(" + arguments[0]->toString() + ")";
        }
        // Binary operator: arg1 op arg2
        if (arguments.size() == 2) {
            return "("+arguments[0]->toString() + " " + op + " " + arguments[1]->toString()+")";
        }
        
        std::string str = "(";
        for (const auto& arg : arguments)
        {
            str += arg->toString() + " ";
        }
        str += ")";
        return str;
    }

    double evaluate(const Environment& env) const override {
        // Evaluate all child nodes first, this ensures nested operations and functions get evaluated first
        std::vector<double> evaluatedArgs;
        for (const auto& child : arguments) {
            evaluatedArgs.push_back(child->evaluate(env));
        }

        // Look up in the registry and calculate the final result
        return procedureRegistry.execute(op, evaluatedArgs);
    }
};



