#include "Node.hpp"

inline std::ostream& operator <<(std::ostream& out, const AST::Environment& env)
{
    std::map<std::string, double> sorted_env(env.begin(), env.end());
    for (const auto& pair : sorted_env)
    {
        out << pair.first << ": " << pair.second << '\n';
    }
    return out;
}