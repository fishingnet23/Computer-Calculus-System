#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <map>

namespace AST
{
using Environment = std::unordered_map<std::string, double>;

void static printEnvironment(const Environment& env)
{
    std::map<std::string, double> sorted_env(env.begin(), env.end());
    for (const auto& pair : sorted_env)
    {
        std::cout << pair.first << ": " << pair.second << '\n';
    }
}

class Node {
public:
    virtual ~Node() = default;

    virtual std::string toString() const = 0;
    virtual double evaluate(const Environment& env) const = 0;
};

using NodePtr = std::shared_ptr<Node>;

};