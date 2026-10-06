#include "AST.hpp"
namespace AST
{
NodePtr Tree::parseTokens(const std::vector<Token> &tokens, const OperationRegistry &registry)
{
    Parser parser(tokens, registry);
    return parser.parseTokens();
}

Tree::Tree(const std::string &src, const OperationRegistry &registry)
{
    Tokenizer tokenizer(src);
    auto tokens = tokenizer.tokenize();
    root = parseTokens(tokens, registry);
}
};
