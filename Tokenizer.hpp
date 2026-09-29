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

	Token(TokenType t, std::string v) :type(t), value(v) {}
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

public:

	Tokenizer(std::string src)
	{
		this->src = src;
		this->pos = 0;
	}

	void setSrc(std::string src){this->src = src;}
	std::string getSrc() const { return src; }

    Token next()
    {
        // skip whitespace
        while (pos < src.length() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r'))
        {
            pos++;
        }

        if (pos >= src.length())
        {
            // the last token was just whitespace, just send an empty token for the parser to deal with
            return Token(TokenType::UNKNOWN, "");
        }

        char ch = src[pos];

        // delimiters, always single characters
        if (ch == '(')
        {
            pos++;
            return Token(TokenType::L_BRACKET, "(");
        }
        if (ch == ')')
        {
            pos++;
            return Token(TokenType::R_BRACKET, ")");
        }

        // multicharacter literals
        if ((ch >= '0' && ch <= '9') || ch == '.')
        {
            std::string token = "";
            while (pos < src.length() && ((src[pos] >= '0' && src[pos] <= '9') || src[pos] == '.'))
            {
                token += src[pos];
                pos++;
            }
            return Token(TokenType::LITERAL, token);
        }

        // multicharacter identifiers
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
        {
            std::string token = "";
            while (pos < src.length() && ((src[pos] >= 'a' && src[pos] <= 'z') ||
                (src[pos] >= 'A' && src[pos] <= 'Z') ||
                (src[pos] >= '0' && src[pos] <= '9') ||
                src[pos] == '_'))
            {
                token += src[pos];
                pos++;
            }
            return Token(TokenType::IDENTIFIER, token);
        }

        // unknowns (operators/symbols),scan contiguous nonalphanumeric characters
        std::string token = "";
        while (pos < src.length())
        {
            char current = src[pos];

            // Stop scanning operator cluster if we hit whitespace, brackets, digits, or letters
            if (current == ' ' || current == '\t' || current == '\n' || current == '\r' ||
                current == '(' || current == ')' ||
                (current >= '0' && current <= '9') ||
                (current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z') || current == '_')
            {
                break;
            }

            token += current;
            pos++;

        }

        return Token(TokenType::UNKNOWN, token);
    }

	std::vector<Token> tokenize()
	{
		pos = 0;
		std::vector<Token> res;
		while (pos < src.length())
		{
			Token nxt = next();
			res.push_back(nxt);
		}
		return res;
	}
	
};

