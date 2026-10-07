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
    explicit Environment(const std::vector<std::string>& tokens)
    {
        for(const std::string& arg:tokens)
        {
            data[arg] = 0.0;
        }
    }
    double& operator[](const std::string& token)
    {
        return data[token];
    }
    void print()
    {
        std::map<std::string, double> sorted_env(data.begin(), data.end());
        for (const auto& pair : sorted_env)
        {
            std::cout << pair.first << ": " << pair.second << '\n';
        }
    }
}




class Node {
    static constexpr int MAX_SIMPLIFICATION_STEPS = 50; // Maximum number of simplification steps to avoid infinite loops
protected:
    std::vector<std::shared_ptr<Node>> arguments;
    Node* father;
public:
    virtual ~Node() = default;
    virtual std::string toString() const = 0;
    virtual std::shared_ptr<Node> simplifyStep() = 0;
    virtual double evaluate(const Environment& env) const = 0;
    std::shared_ptr<Node> simplify();

    void setFatherNode(Node* parent) { father = parent; }
    Node* getFatherNode() const { return father; }
    bool virtual equals(const Node* other) const = 0;
};

using NodePtr = std::shared_ptr<Node>;


};