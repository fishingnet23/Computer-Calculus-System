#include "Parser.hpp"


namespace AST
{

Parser::Parser(const std::vector<Token>& tokens):tokens(tokens){}

void Parser::finalizeOperator(std::vector<NodePtr>& output, std::vector<Parser::opStackData>& operators)
{
    auto& last = operators.back();
    auto opDefinition = OpRegistry::get(last.token.value);
    if(!opDefinition) // if operators was made correct, not needed, extra safety
        throw std::runtime_error("Unknown operator! " + last.token.value);
    
    int argumentCount = opDefinition->getCanonArgumentCount();
    std::vector<NodePtr> arguments = {};
    arguments.reserve(argumentCount);

    for(int i=argumentCount-1;i > -1;i--)
    {
        arguments[i] = output.back();
        output.pop_back();
    }

    NodePtr op = NodeFactory::makeOperator(last.token.value, arguments);
    output.push_back(op);
    operators.pop_back();

}



std::vector<NodePtr> Parser::parseTokens() const
{
    std::vector<NodePtr> output;
    std::vector<opStackData> operators;

    for(const auto& token : tokens)
    {
        if(token.type == TokenType::NUMBER)
            output.push_back(NodeFactory::makeNumber(token.value));
        else if(token.type == TokenType::VARIABLE)
            output.push_back(NodeFactory::makeVariable(token.value));
        else if(token.type == TokenType::L_BRACKET)
            operators.push_back({token,0});
        else if(token.type == TokenType::R_BRACKET)
        {
            while(!operators.empty() && operators.back().token.type != TokenType::L_BRACKET)
            {
                finalizeOperator(output,operators);
            }
            if(operators.empty())
                throw std::runtime_error("Missmatched brackets!");
            operators.pop_back();
        }
        else if(token.type == TokenType::OPERATION)
        {
            auto opDefinition = OpRegistry::get(token.value);
            if(!opDefinition)
                throw std::runtime_error("Unknown operator! " + token.value);

            while(!operators.empty())
            {    
                if(operators.back().token.type == TokenType::L_BRACKET) // not a real operator, just a syntax marker
                    break;

                int topPrecedence = operators.back().precedence;
                int incomingPrecedence = opDefinition->getPrecedence();
                bool isRightAssoc = opDefinition->hasProperty(OpProperty::RIGHT_ASSOCIATIVE);

                if ((!isRightAssoc && topPrecedence >= incomingPrecedence) || 
                    (isRightAssoc && topPrecedence > incomingPrecedence))
                    finalizeOperator(output, operators);
                else
                    break;
            }
            operators.push_back({token, opDefinition->getPrecedence()});
        }
    }

    // final build
    while(!operators.empty()) 
    {
    if(operators.back().token.type == TokenType::L_BRACKET)
        throw std::runtime_error("Mismatched brackets!");
        
    finalizeOperator(output, operators);
    }

    return output;
}
NodePtr Parser::buildAST() const
{
    std::vector<NodePtr> finalAST = parseTokens();

    if(finalAST.size() != 1)
        throw std::runtime_error("Malformed Syntax Tree! Operations and arguments were entered too many, too little or wrongly. Ensure correct math syntax!");
    return finalAST[0]
}
};