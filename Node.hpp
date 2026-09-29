#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

// Dictionary of variable values
using Environment = std::unordered_map<std::string, double>;

class Node {
public:
    virtual ~Node() = default;

    virtual std::string toString() const = 0;
    virtual double evaluate(const Environment& env) const = 0;
};

// convenience alias
using NodePtr = std::shared_ptr<Node>;
