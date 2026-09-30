#include "Parser.hpp"

namespace AST
{
class ASTTree
{
    ASTNodePtr root;
    ASTNodePtr parseTokens(const std::vector<Token>& tokens, const OperationRegistry& registry)
    {
        Parser parser(tokens, registry);
        return parser.parseTokens();
    }
public:
    ASTTree(ASTNodePtr root) :root(root) {}
    ASTTree() :root(nullptr) {}
    ASTTree(const std::vector<Token>& tokens, const OperationRegistry& registry)
    {
        root = parseTokens(tokens, registry);
    }
    ASTTree(const std::string& src, const OperationRegistry& registry)
    {
        Tokenizer tokenizer(src);
        auto tokens = tokenizer.tokenize();
        root = parseTokens(tokens, registry);
    }

    ASTNodePtr getRoot() const { return root; }
    void setRoot(ASTNodePtr newRoot) { root = newRoot;}

    std::string toString() const
    {
        if (root)
            return root->toString();
        else
            return "";
    }

    double evaluate(const Environment& env) const
    {
        if (root)
            return root->evaluate(env);
        else
            return 0.0;
    }


};
}