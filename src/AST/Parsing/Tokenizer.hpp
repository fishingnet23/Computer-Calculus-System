#pragma once
#include <string>
#include "../include/NodeFactory.hpp"
#include <limits>
#include <iostream>

namespace AST
{
enum class TokenType
{
	UNKNOWN,
	L_BRACKET,
	R_BRACKET,
	COMMA,
	VARIABLE,
	NUMBER,
	OPERATION,
};

class Token
{
public:
	TokenType type;
	OpDefinition::Format format;
	std::string value;

	Token(TokenType t, const std::string& v, const OpDefinition::Format& f) :type(t), value(v),format(f) {}
	Token(TokenType t, const std::string& v) :type(t), value(v),format(OpDefinition::Format::NONE) {}

};

inline std::ostream& operator <<(std::ostream& out, const TokenType& type)
{

	if (type == TokenType::L_BRACKET)
		out << "L_BRACKET";
	else if (type == TokenType::R_BRACKET)
		out << "R_BRACKET";
	else if (type == TokenType::COMMA)
		out << "COMMA";
	else if (type == TokenType::VARIABLE)
		out << "VARIABLE";
	else if (type == TokenType::NUMBER)
		out << "NUMBER";
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
};
