#pragma once
#include"Tokenizer.hpp"
#include"../Structure/Nodes.hpp"



namespace AST
{
class Parser
{
private:
	const std::vector<Token>& tokens;
	struct OperatorContext
	{
		const Token* token;
		std::shared_ptr<const OpDefinition> op;
		size_t index;
		int precedence;
	};
	OperatorContext getRootOperator(size_t start, size_t end);
public:
	Parser(const std::vector<Token>& tokens);



	NodePtr parseTokens();
	bool isWrappedInBrackets(size_t start, size_t end);
	NodePtr parseTokens(size_t contextStart, size_t contextEnd);
};
};