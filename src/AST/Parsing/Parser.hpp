#pragma once
#include"Tokenizer.hpp"
#include"../Structure/Nodes.hpp"



namespace AST
{
class Parser
{
private:
	const std::vector<Token>& tokens;

	struct opStackData // used over a direct op definition because it needs to support things that dont have to live on the AST too
	{
		const Token& token;
		size_t precedence;

	};

public:
	Parser(const std::vector<Token>& tokens);

	std::vector<NodePtr> parseTokens() const;
	static void finalizeOperator(std::vector<NodePtr>& output, std::vector<Parser::opStackData>& operators); // pops operators, builds from arguments and reinserts into output
	NodePtr buildAST() const; // wrapper function, outside callers dont care about the intermediate step
};
};