#include "Tokenizer.hpp"

void Tokenizer::skipWhitespace()
{
    while (pos < src.length() && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r'))
    {
        pos++;
    }
}
Token Tokenizer::extractLiteral()
{
    std::string token = "";
    while (pos < src.length() && ((src[pos] >= '0' && src[pos] <= '9') || src[pos] == '.'))
    {
        token += src[pos];
        pos++;
    }
    return Token(TokenType::LITERAL, token);
}

Token Tokenizer::extractIdentifier()
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
Token Tokenizer::extractUnknown()
{
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
Token Tokenizer::next()
{
    skipWhitespace();

    // the last token was just whitespace, just send an empty token for the parser to deal with
    if (pos >= src.length())
        return Token(TokenType::UNKNOWN, "");

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
        return extractLiteral();
    }

    // multicharacter identifiers
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
    {
        return extractIdentifier();
    }

    return extractUnknown();
}

std::vector<Token> Tokenizer::tokenize()
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

