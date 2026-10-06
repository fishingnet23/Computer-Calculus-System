#include "Node.hpp"
namespace AST
{
inline std::ostream& operator <<(std::ostream& out, const AST::Environment& env)
{
    std::map<std::string, double> sorted_env(env.begin(), env.end());
    for (const auto& pair : sorted_env)
    {
        out << pair.first << ": " << pair.second << '\n';
    }
    return out;
}

std::shared_ptr<Node> Node::simplify()
{
    size_t maximumLength = this->toString().length();
    std::shared_ptr<Node> simplifiedNode = this->simplifyStep();
    std::cout << "Simplification pass 1: " << simplifiedNode->toString() << "\n";

    for(int i=1;i<MAX_SIMPLIFICATION_STEPS;i++) // limit the number of simplification steps to avoid infinite loops
    {
        if(simplifiedNode->toString().length() >= maximumLength) // if the simplification step didn't reduce the length of the expression, stop simplifying
        {
            std::cout << "not simpler after pass " << i << ". stopping simplification.\n";    
            return simplifiedNode;
        }
        else
            maximumLength = simplifiedNode->toString().length();
        simplifiedNode = simplifiedNode->simplifyStep();
        std::cout << "simplification pass " << i+1 << ": " << simplifiedNode->toString() << "\n";
        
    }
    return simplifiedNode->simplifyStep(); // try one last time, let simplify step handle what to do if it can't simplify further
}
};