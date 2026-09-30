#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>


// Dictionary of variable values
using Environment = std::unordered_map<std::string, double>;

#include <iostream>
#include <unordered_map>
#include <map>
#include <string>

using Environment = std::unordered_map<std::string, double>;

inline std::ostream& operator <<(std::ostream& out, const Environment& env)
{
    std::map<std::string, double> sorted_env(env.begin(), env.end());

    for (const auto& pair : sorted_env)
    {
        out << pair.first << ": " << pair.second << '\n';
    }
    return out;
}


class ASTNode {
public:
    virtual ~ASTNode() = default;

    virtual std::string toString() const = 0;
    virtual double evaluate(const Environment& env) const = 0;
};

// convenience alias
using ASTNodePtr = std::shared_ptr<ASTNode>;
