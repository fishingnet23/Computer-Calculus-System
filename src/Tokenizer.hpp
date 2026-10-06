#pragma once
#include <string>
#include "Operators.hpp"
#include <limits>
#include <iostream>

namespace AST
{
enum class TokenType
{
	UNKNOWN = -1,
	L_BRACKET = 0,
	R_BRACKET = 1,
	IDENTIFIER = 2,
	LITERAL = 3,
	OPERATION = 4,
};

class Token
{
public:
	TokenType type;
	std::string value;
	Token(TokenType t, const std::string& v) :type(t), value(v) {}
};

inline std::ostream& operator <<(std::ostream& out, const TokenType& type)
{

	if (type == TokenType::L_BRACKET)
		out << "L_BRACKET";
	else if (type == TokenType::R_BRACKET)
		out << "R_BRACKET";
	else if (type == TokenType::IDENTIFIER)
		out << "IDENTIFIER";
	else if (type == TokenType::LITERAL)
		out << "LITERAL";
	else if (type == TokenType::OPERATION)
		out	<< "OPERATION";
	else
		out << "UNKNOWN";
	return out;
}

inline std::ostream& operator <<(std::ostream& out,const Token& token)
{
	return out << "[" << token.value << ", " << token.type << "]";
}

class Tokenizer
{

private:
	std::string src;
	std::size_t pos = 0;

	const OperationRegistry& registry;

	void skipWhitespace();
	
    Token extractLiteral();
	Token extractIdentifier();
	Token extractUnknown();
public:

	Tokenizer(const std::string& src,const OperationRegistry& registry):registry(registry)
	{
		this->src = src;
		this->pos = 0;
	}

    Tokenizer(const std::string& src, int p, const OperationRegistry& registry):src(src),pos(p),registry(registry){}

	void setSrc(const std::string& src){this->src = src;}
	std::string getSrc() const { return src; }

    Token next();

    std::vector<Token> tokenize();
};
};
