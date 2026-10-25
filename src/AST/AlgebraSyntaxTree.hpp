#include "Parsing/Parser.hpp"

namespace AST
{
class AlgebraExpression
{
    NodePtr root;
    NodePtr parseTokens(const std::vector<Token>& tokens);
public:
    explicit AlgebraExpression(NodePtr root) :root(root) {}
    AlgebraExpression() :root(nullptr) {}
    explicit AlgebraExpression(const std::vector<Token>& tokens);
    explicit AlgebraExpression(const std::string& src);

    void buildFrom(const std::string& src);
    void buildFrom(const std::vector<Token>& tokens){root = parseTokens(tokens);}

    NodePtr getRoot() const { return root; }
    void setRoot(NodePtr newRoot) { root = newRoot;}

    std::string toString() const
    {
        if (root)
            return root->toString();
        else
            return "EMPTY TREE";
    }

    std::string postfixNotation()
    {
        if(root)
            return root->getPostFix();
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
        auto* symbol = root->asSymbol();
        if(!symbol)
            return;
        auto* op = symbol->asOperator();
        if(!op)
            return;
            
        // Anchor the result locally first
        NodePtr newRoot = op->simplify();
        root = std::move(newRoot);
    }

    void expand()
    {
        if(!root)
            return;
        auto* symbol = root->asSymbol();
        if(!symbol)
            return;
        auto* op = symbol->asOperator();
        if(!op)
            return;
            
        // Anchor the result locally first
        NodePtr newRoot = op->expand();
        root = std::move(newRoot);
    }


    void cannonize()
    {
        expand();
        compress();
    }
};
}