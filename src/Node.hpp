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
};




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
    virtual std::string getName() const = 0;
    Node* getFatherNode() const { return father; }
    bool virtual equals(const Node* other) const = 0;
    const std::vector<std::shared_ptr<Node>>& getArguments() const {return arguments;}

    static int nodeRank(const std::shared_ptr<Node>& node);
    static bool canonicalLess(const std::shared_ptr<Node>& a, const std::shared_ptr<Node>& b);
};

using NodePtr = std::shared_ptr<Node>;




};