#pragma once
#include"Tokenizer.hpp"
#include"TreeNodes.h"



namespace AST
{
class Parser
{
private:
	const std::vector<Token>& tokens;
	const OperationRegistry& registry;	
	struct OperatorContext
	{
		const Token* token;
		const OpDefiniton* op;
		size_t index;
		int precedence;
	};
	OperatorContext getRootOperator(size_t start, size_t end);
public:
	Parser(const std::vector<Token>& tokens, const OperationRegistry& registry);



	NodePtr parseTokens();
	bool isWrappedInBrackets(size_t start, size_t end);
	NodePtr parseTokens(size_t contextStart, size_t contextEnd);
};
};