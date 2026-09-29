#pragma once
#include <string>
#include "Operators.hpp"
#include <limits>
#include <iostream>

enum class TokenType
{
	UNKNOWN = -1,
	L_BRACKET = 0,
	R_BRACKET = 1,
	IDENTIFIER = 2,
	LITERAL = 3,
	
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

	void skipWhitespace();
	
    Token extractLiteral();
	Token extractIdentifier();
	Token extractUnknown();
public:

	Tokenizer(const std::string& src)
	{
		this->src = src;
		this->pos = 0;
	}

    Tokenizer(const std::string& src, int p):src(src),pos(p){}

	void setSrc(const std::string& src){this->src = src;}
	std::string getSrc() const { return src; }

    Token next();

    std::vector<Token> tokenize();
};

