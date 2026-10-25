#include "Parser.hpp"

namespace AST
{
class AlgebraExpression
{
    NodePtr root;
    NodePtr parseTokens(const std::vector<Token>& tokens, const OperationRegistry& registry);
public:
    AlgebraExpression(NodePtr root) :root(root) {}
    AlgebraExpression() :root(nullptr) {}
    AlgebraExpression(const std::vector<Token>& tokens, const OperationRegistry& registry){root = parseTokens(tokens, registry);}
    AlgebraExpression(const std::string& src, const OperationRegistry& registry);

    NodePtr getRoot() const { return root; }
    void setRoot(NodePtr newRoot) { root = newRoot;}

    std::string toString() const
    {
        if (root)
            return root->toString();
        else
            return "EMPTY TREE";
    }

    double evaluate(const Environment& env) const
    {
        if (root)
            return root->evaluate(env);
        else
            return 0.0;
    }

    void compress()
    {
        if(!root)
            return;
        const auto* symbol = root->asSymbol();
        if(!symbol)
            return;
        root = symbol->simplify();
    }
};
}