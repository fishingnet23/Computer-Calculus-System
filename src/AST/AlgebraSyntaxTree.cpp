#include "AlgebraSyntaxTree.hpp"
namespace AST
{
NodePtr AlgebraExpression::parseTokens(const std::vector<Token> &tokens)
{
    Parser parser(tokens);
    return parser.buildAST();
}
AlgebraExpression::AlgebraExpression(const std::vector<Token> &tokens)
{
    buildFrom(tokens);
}

AlgebraExpression::AlgebraExpression(const std::string &src)
{
    buildFrom(src);
}
void AlgebraExpression::buildFrom(const std::string &src)
{
    Tokenizer tokenizer(src);
    auto tokens = tokenizer.tokenize();
    root = parseTokens(tokens);
}
};
