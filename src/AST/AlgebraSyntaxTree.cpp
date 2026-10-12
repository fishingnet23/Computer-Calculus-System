#include "AlgebraSyntaxTree.hpp"
namespace AST
{
NodePtr Tree::parseTokens(const std::vector<Token> &tokens)
{
    Parser parser(tokens);
    return parser.parseTokens();
}
Tree::Tree(const std::vector<Token> &tokens)
{
    buildFrom(tokens);
}

Tree::Tree(const std::string &src)
{
    buildFrom(src);
}
void Tree::buildFrom(const std::string &src)
{
    Tokenizer tokenizer(src);
    auto tokens = tokenizer.tokenize();
    root = parseTokens(tokens);
}
};
