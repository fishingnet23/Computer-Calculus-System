#include "Node.hpp"
#include "TreeNodes.h"
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

std::shared_ptr<Node> Symbol::simplify() const
{
    size_t maximumLength = this->toString().length();
    std::cout << "presimplification: " << this->toString() << "\n";
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
        if(const Symbol* symbol = simplifiedNode->asSymbol())
        {
            simplifiedNode = symbol->simplifyStep();
        }
        
        std::cout << "simplification pass " << i+1 << ": " << simplifiedNode->toString() << "\n";
        
    }
    return simplifiedNode;
}
Environment::Environment(const std::vector<std::string> &tokens)
{
    for(size_t i=0;i<tokens.size();i++)
    {
        const std::string& arg = tokens[i];
        double val = 0.0;
        if(i+1 < tokens.size())
        {
            try
            {
                val = std::stod(tokens[i+1]);
            }
            catch (const std::exception& e) {
                data[arg] = 0.0;
                continue;
            }
            data[arg] = val;
            i++;
        }
        else
            try
            {
                val = std::stod(arg);
                break;
            }
            catch (const std::exception& e) {
                data[arg] = 0.0;
                break;
            }

    }
}
int Node::nodeRank(const std::shared_ptr<Node> &node)
{
    if (node->type == NodeType::Number)
        return 0;

    if (node->type == NodeType::Variable)
        return 1;

    if (node->type == NodeType::Operator)
        return 2;

    return 3;
}
bool Node::canonicalLess(const std::shared_ptr<Node> &a, const std::shared_ptr<Node> &b)
{
    int rankA = nodeRank(a);
    int rankB = nodeRank(b);

    if (rankA != rankB)
        return rankA < rankB;

    if(a->type != NodeType::Variable || b->type != NodeType::Variable)
        return a->toString() < b->toString();

    const auto* varA = static_cast<const VariableNode*>(a.get());
    const auto* varB = static_cast<const VariableNode*>(b.get());

    if (varA->getName() != varB->getName())
        return varA->getName() < varB->getName();

    if (varA->getDegree() != varB->getDegree())
        return varA->getDegree() < varB->getDegree();

    return varA->getCoeffecient() < varB->getCoeffecient();

}
};