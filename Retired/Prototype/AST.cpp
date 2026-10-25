#include "AST.hpp"
namespace AST
{
NodePtr AlgebraExpression::parseTokens(const std::vector<Token> &tokens, const OperationRegistry &registry)
{
    Parser parser(tokens, registry);
    return parser.parseTokens();
}

AlgebraExpression::AlgebraExpression(const std::string &src, const OperationRegistry &registry)
{
    Tokenizer tokenizer(src,registry);
    auto tokens = tokenizer.tokenize();
    root = parseTokens(tokens, registry);
}
};
