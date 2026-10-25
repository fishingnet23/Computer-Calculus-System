#include "Parser.hpp"


namespace AST
{

Parser::Parser(const std::vector<Token>& tokens):tokens(tokens){}

void Parser::finalizeOperator(std::vector<NodePtr>& output, std::vector<Parser::opStackData>& operators)
{
    auto& last = operators.back();
    auto syntaxRule = OpRegistry::getRule(last.token.value);

    if(!syntaxRule) // if operators was made correct, not needed, extra safety
        throw std::runtime_error("Unknown operator! " + last.token.value);
    
    int argumentCount = (last.token.format == OpDefinition::Format::EXPLICIT_FUNCTION) 
                        ? last.argumentCount 
                        : syntaxRule->getCanonArgumentCount();
    std::vector<NodePtr> arguments(argumentCount);

    for(int i=argumentCount-1;i >= 0;i--)
    {
        if(output.empty())
            throw std::runtime_error("Invalid syntax!");

        arguments[i] = output.back();
        if(Config::DEBUG_MODE) std::cout << arguments[i]->toString() << std::endl;
        output.pop_back();
    }

    
    OpPtr op = NodeFactory::makeOperatorPtr(last.token.value, arguments);
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
        {
            output.push_back(NodeFactory::makeNumber(token.value));
        }    
        else if(token.type == TokenType::VARIABLE)
        {
            output.push_back(NodeFactory::makeVariable(token.value));
        }
        else if(token.type == TokenType::L_BRACKET)
        {
            operators.push_back({token,0,1});
        }
        else if(token.type == TokenType::COMMA)
        {
            // Pop all operators in this slice back down to the enclosing left bracket
            while(!operators.empty() && operators.back().token.type != TokenType::L_BRACKET)
            {
                finalizeOperator(output, operators);
            }
            
            if(operators.empty())
                throw std::runtime_error("Mismatched comma! No enclosing brackets found.");
                
            if (operators.size() >= 2)
            {
                auto& potentialFunc = operators[operators.size() - 2];
                if (potentialFunc.token.format == OpDefinition::Format::EXPLICIT_FUNCTION)
                {
                    potentialFunc.argumentCount++;
                }
            }
        }
        else if(token.type == TokenType::R_BRACKET)
        {
            while(!operators.empty() && operators.back().token.type != TokenType::L_BRACKET)
            {
                finalizeOperator(output,operators);
            }
            if(operators.empty())
                throw std::runtime_error("Missmatched brackets!");
            operators.pop_back();

            // if the brackets were sitting after an explicit function, like sin(x), it must be evaluated now
            // this is to force f(x)^2 = (f(x))^2 and not f(x^2), which is actually desireable when f is "-", as in -(x)^2 = -(x^2)
            if (!operators.empty() && operators.back().token.format == OpDefinition::Format::EXPLICIT_FUNCTION)
            {
                finalizeOperator(output, operators);
            }
        }
        else if(token.type == TokenType::OPERATION)
        {

            auto syntaxRule = OpRegistry::getRule(token.value);
            if(!syntaxRule)
                throw std::runtime_error("Unknown operator! " + token.value);

            size_t incomingPrecedence = syntaxRule->getPrecedence();
            bool isRightAssoc = syntaxRule->hasProperty(OpProperty::RIGHT_ASSOCIATIVE);

            while(!operators.empty())
            {   
                const auto& topOp = operators.back();

                if(topOp.token.type == TokenType::L_BRACKET) // cannot escape bracket context
                    break;

                size_t topPrecedence = topOp.precedence;
                
                // if the incoming operator is infix but a prefix is on the stack, precedence has to be strictly higher
                // this is to ensure -x^2 becomes -(x^2) but -x+3 becomes (-x)+3
                if (token.format == OpDefinition::Format::INFIX && topOp.token.format == OpDefinition::Format::PREFIX) 
                {

                    if (incomingPrecedence > topPrecedence)
                        break;
                    else
                    {
                        finalizeOperator(output, operators);
                        continue;
                    }
                }
                else if (token.format == topOp.token.format) // if the formats are equal, standard comparison
                {
                    if (!((!isRightAssoc && topPrecedence >= incomingPrecedence) || 
                          (isRightAssoc && topPrecedence > incomingPrecedence)))
                    {
                        break;
                    }
                }
                else if(topOp.token.format == OpDefinition::Format::EXPLICIT_FUNCTION) 
                // if the explicit function is visible, it means there were no brackets, which means it must bind now
                {
                    finalizeOperator(output, operators);
                    continue;
                }
                // if a prefix finds an infix already on the stack, it always comes first.
                // this is to allow cases like x^-1, where the prefix is inside the context of an infix
                else 
                {
                    break;
                }

                finalizeOperator(output, operators);
            }
            
            operators.push_back({token, incomingPrecedence,1});
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
    return finalAST[0];
}
};