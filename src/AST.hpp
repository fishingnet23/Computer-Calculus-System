#include "Parser.hpp"

namespace AST
{
class Tree
{
    NodePtr root;
    NodePtr parseTokens(const std::vector<Token>& tokens, const OperationRegistry& registry);
public:
    Tree(NodePtr root) :root(root) {}
    Tree() :root(nullptr) {}
    Tree(const std::vector<Token>& tokens, const OperationRegistry& registry){root = parseTokens(tokens, registry);}
    Tree(const std::string& src, const OperationRegistry& registry);

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

    void simplify()
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