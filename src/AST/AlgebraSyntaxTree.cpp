#include "../../include/AlgebraSyntaxTree.hpp"
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
    unify();
}
void AlgebraExpression::buildFrom(const std::string &src)
{
    Tokenizer tokenizer(src);
    auto tokens = tokenizer.tokenize();
    root = parseTokens(tokens);
}
NodePtr AlgebraExpression::findVariableNode(const std::string &variableName)
{
    if (!root)
        return nullptr;

    auto sym = SoftCasting::toSymbol(root);
    if (sym && sym->getName() == variableName)
        return root;

    auto op = SoftCasting::toOperator(root);
    if (op)
    {
        for (const auto &arg : op->getArguments())
        {
            auto foundNode = AlgebraExpression(arg).findVariableNode(variableName);
            if (foundNode)
                return foundNode;
        }
    }

    return nullptr;
}
};
