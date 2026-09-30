#include "Parser.hpp"


namespace AST
{
Parser::OperatorContext Parser::getRootOperator(size_t start, size_t end)
{
    OperatorContext result{ nullptr, nullptr,0 };

    int depth = 0;
    int bestPriority = std::numeric_limits<int>::max();

    if (tokens.size() < end || start > tokens.size() || start >= end)
        throw std::runtime_error("out of bounds!");

    for (size_t i = start; i < end; ++i)
    {
        const Token& token = tokens[i];

        if (token.type == TokenType::L_BRACKET)
        {
            ++depth;
            continue;
        }

        if (token.type == TokenType::R_BRACKET)
        {
            --depth;
            continue;
        }

        // operators inside brackets must be ignored at this level
        if (depth != 0)
            continue;

        auto pair = registry.getRegistry().find(token.value);

        // not an operator
        if (pair == registry.getRegistry().end())
            continue;

        int priority = pair->second.precedence;

        if (priority < bestPriority)
        {
            bestPriority = priority;
            result.token = &token;
            result.index = i;
            result.op = &pair->second;
        }
    }

    return result;
}

Parser::Parser(const std::vector<Token>& tokens, const OperationRegistry& registry):tokens(tokens),registry(registry){}

NodePtr Parser::parseTokens()
{
	return parseTokens(0,tokens.size());
}
bool Parser::isWrappedInBrackets(size_t start, size_t end)
{
    if (tokens[start].type != TokenType::L_BRACKET ||
        tokens[end - 1].type != TokenType::R_BRACKET)
        return false;

    int depth = 0;

    for (size_t i = start; i < end; ++i)
    {
        if (tokens[i].type == TokenType::L_BRACKET)
            ++depth;

        else if (tokens[i].type == TokenType::R_BRACKET)
            --depth;

        // The first pair closes before the end,
        // meaning these aren't wrapping the whole expression.
        if (depth == 0 && i != end - 1)
            return false;
    }

    return true;
}
NodePtr Parser::parseTokens(size_t start, size_t end)
{

	// single token case
	if (end - start == 1)
	{
		const Token& token = tokens[start];

		if (token.type == TokenType::LITERAL)
			return std::make_shared<NumberNode>(token.value);

		if (token.type == TokenType::IDENTIFIER)
			return std::make_shared<VariableNode>(token.value);

		throw std::runtime_error("Unexpected token: " + token.value);
	}

    if (isWrappedInBrackets(start, end))
    {
        return parseTokens(start + 1, end - 1);
    }

    OperatorContext root = getRootOperator(start,end);
    NodePtr rootNode;
    if (root.token == nullptr)
    {
        // Range is empty or invalid sub-expression
        if (start >= end)
            throw std::runtime_error("Empty or invalid token range");

        throw std::runtime_error("Unable to parse expression in range [" +
            std::to_string(start) + ", " + std::to_string(end) + "]");
    }

    if (root.op->format == Operation::Format::INFIX)
    {
        NodePtr left = parseTokens(start, root.index);
        NodePtr right = parseTokens(root.index + 1, end);
        rootNode = std::make_shared<OperatorNode>(root.token->value, std::vector<NodePtr>{ left,right },registry);
    }
    else if (root.op->format == Operation::Format::PREFIX)
    {
        NodePtr inner = parseTokens(root.index+1,end);
        rootNode = std::make_shared<OperatorNode>(root.token->value, std::vector<NodePtr>{inner}, registry);
    }
    else if (root.op->format == Operation::Format::POSTFIX)
    {
        NodePtr inner = parseTokens(start, root.index);
        rootNode = std::make_shared<OperatorNode>(root.token->value, std::vector<NodePtr>{ inner }, registry);
    }
    else
    {
        throw std::runtime_error("Not implemented exception!");
    }

    return rootNode;
    
}
};