#include "Parser.hpp"


namespace AST
{
Parser::OperatorContext Parser::getRootOperator(size_t start, size_t end)
{
    OperatorContext result{ nullptr, nullptr,0 };

    int depth = 0;
    int bestPriority = std::numeric_limits<int>::max();

    if (tokens.size() < end || start > tokens.size() || start >= end)
        throw std::runtime_error("out of bounds!" + std::to_string(start) + " " + std::to_string(end) + " " + std::to_string(tokens.size()));

    for (size_t i = start; i < end; ++i)
    {
        const Token& token = tokens[i];

        if ((token.type== TokenType::IDENTIFIER||token.type == TokenType::OPERATION) && (i + 1 < end) && tokens[i + 1].type == TokenType::L_BRACKET)
        {
            auto pair = registry.getRegistry().find(token.value);
            if (pair != registry.getRegistry().end() && pair->second->format == OpDefiniton::Format::EXPLICIT_FUNCTION)
            {
                // if already nested, just let regular depth tracking handle it
                if (depth == 0)
                {
                    int funcDepth = 0;
                    size_t j = i + 1;
                    for (; j < end; ++j)
                    {
                        if (tokens[j].type == TokenType::L_BRACKET) ++funcDepth;
                        else if (tokens[j].type == TokenType::R_BRACKET) --funcDepth;

                        if (funcDepth == 0) break;
                    }
                    // if the function covers the whole outer span, it's the root
                    if (i == start && j == end - 1)
                    {
                        result.token = &token;
                        result.index = i;
                        result.op = pair->second.get();
                        return result;
                    }
                    
                }
            }
        }

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

        int priority = pair->second->precedence;
        const MultiArg_OpDefinition* b_op = dynamic_cast<const MultiArg_OpDefinition*>(pair->second.get());
        bool r_associative = b_op!=nullptr && b_op->hasProperty(MultiArg_OpDefinition::Property::BINARY_RIGHT_ASSOCIATIVE);
        bool l_associative = b_op!=nullptr && b_op->hasProperty(MultiArg_OpDefinition::Property::BINARY_LEFT_ASSOCIATIVE);

        // If the operator is left associative only, we want to consider the right most operator as the root.
        // Otherwise, we can consider the left most operator as the root (implicit right associativity).

        if (priority < bestPriority || (priority == bestPriority && l_associative))
        {
            bestPriority = priority;
            result.token = &token;
            result.index = i;
            result.op = pair->second.get();
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
        if (depth <= 0 && i != end - 1)
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

		if (token.type == TokenType::IDENTIFIER || token.type == TokenType::OPERATION)
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

    if (root.op->format == OpDefiniton::Format::INFIX)
    {
        // This is a binary operator, so we will try to apply smart parsing rules like associativity and precedence.
        NodePtr left = parseTokens(start, root.index);
        NodePtr right = parseTokens(root.index + 1, end);
        const MultiArg_OpDefinition* b_op = dynamic_cast<const MultiArg_OpDefinition*>(root.op);
        std::vector<NodePtr> args;

        if (b_op && b_op->hasProperty(MultiArg_OpDefinition::Property::COMMUTATIVE))
        {
            // If the operator is commutative, we can sort the arguments to make the tree more canonical.
            // This is useful for simplification and comparison of expressions.
            if (left->toString() > right->toString())
                args = { right,left };
            else
                args = { left,right };
        }
        else
        {
            args = { left,right };
        }
        rootNode = std::make_shared<OperatorNode>(root.token->value, args,registry);
    }   
    else if (root.op->format == OpDefiniton::Format::PREFIX)
    {
        NodePtr inner = parseTokens(root.index+1,end);
        rootNode = std::make_shared<OperatorNode>(root.token->value, std::vector<NodePtr>{inner}, registry);
    }
    else if (root.op->format == OpDefiniton::Format::POSTFIX)
    {
        NodePtr inner = parseTokens(start, root.index);
        rootNode = std::make_shared<OperatorNode>(root.token->value, std::vector<NodePtr>{ inner }, registry);
    }
    else if(root.op->format == OpDefiniton::Format::EXPLICIT_FUNCTION)
    {
        if ((root.index + 2 >= end) || (tokens[root.index + 1].type != TokenType::L_BRACKET))
        {
            throw std::runtime_error("Explicit functions require brackets!");
        }

        std::vector<NodePtr> arguments = {};
        arguments.reserve(3);
        
        size_t subStart = root.index + 2; 
        
        int localDepth = 0;
        
        // Loop through all tokens inside the function window
        for (size_t i = root.index + 2; i < end; i++)
        {
            
            if (tokens[i].type == TokenType::L_BRACKET)
            {
                ++localDepth;
            }
            else if (tokens[i].type == TokenType::R_BRACKET)
            {
                --localDepth;
            }

            // 1. If we find a top-level comma, split the argument
            if (tokens[i].value == "," && localDepth == 0)
            {
                size_t subEnd = i;
                if (subStart < subEnd) {
                    arguments.push_back(parseTokens(subStart, subEnd));
                }
                subStart = i + 1;
            }
            // 2. If localDepth hits -1, we found the EXACT closing bracket for THIS function
            else if (localDepth == -1)
            {
                size_t subEnd = i;
                if (subStart < subEnd) {
                    arguments.push_back(parseTokens(subStart, subEnd));
                }
                break; // Safely stop processing arguments for this function level
            }
        }
        
        rootNode = std::make_shared<OperatorNode>(root.token->value, arguments, registry);
    }

    else
    {
        throw std::runtime_error("Not implemented exception!");
    }

    return rootNode;
    
}
};