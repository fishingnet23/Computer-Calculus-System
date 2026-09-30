#include <iostream>
#include "TreeNodes.h"
#include "Tokenizer.hpp"
#include "Parser.hpp"

constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;


static void testToken(std::string src)
{


    AST::Tokenizer t(src);
    std::cout << src << std::endl;
    auto res1 = t.tokenize();
    std::cout << "TOKENIZED: (NOT PARSED, JUST TURNED FROM TEXT INTO TOKENS)" << std::endl;
    for (const auto& word : res1)
        std::cout << word << " , ";
    std::cout << "\n"<<std::endl;
}

static void tokenizerTest()
{

    testToken("sin(x) + 5.233 + 3 + 2");
    testToken("ln(cos(x + 2) * 3) * 3 + 23");
    testToken("\n x^2 / 23 * 100 + sin(x*x*x*x)\n + 532");

}

static void syntaxTreeTest()
{
    AST::OperationRegistry registry;

    AST::NodePtr xNode = std::make_shared<AST::VariableNode>("x");

    AST::NodePtr sinX = std::make_shared<AST::OperatorNode>("sin", std::vector<AST::NodePtr>{xNode}, registry);
    AST::NodePtr cosX = std::make_shared<AST::OperatorNode>("cos", std::vector<AST::NodePtr>{xNode}, registry);
    AST::NodePtr lnX = std::make_shared<AST::OperatorNode>("ln", std::vector<AST::NodePtr>{xNode}, registry);


    AST::NodePtr totalExpr = std::make_shared<AST::OperatorNode>("+", std::vector<AST::NodePtr>{sinX, cosX}, registry);

    std::cout << "Expression: " << totalExpr->toString() << "\n";

    AST::Environment env;
    env["x"] = 0.0;
    std::cout << "Result when x=0: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 1.0;
    std::cout << "Result when x=1: " << totalExpr->evaluate(env) << "\n";
    env["x"] = E;
    std::cout << "Result when x=e: " << totalExpr->evaluate(env) << "\n";
    env["x"] = PI;
    std::cout << "Result when x=PI: " << totalExpr->evaluate(env) << "\n";
}

static void fullTest(const std::string& text, AST::Environment& env)
{
    AST::OperationRegistry registry;

    AST::Tokenizer t(text);
    auto tokens = t.tokenize();
    AST::Parser p(tokens,registry);

    AST::NodePtr parsedExpression = p.parseTokens()->simplify();

    std::cout << "Source Expression: " << text << "\n";
    std::cout << "Parsed Expression: " << parsedExpression->toString() << "\n";

    std::cout << "Enviornment: \n";
    AST::printEnvironment(env);

    std::cout << "Evaluation: " << parsedExpression->evaluate(env) <<"\n"<< std::endl;
}

int main() {
    std::cout << "\n\nTOKENIZER TEST: \n\n" << std::endl;
    tokenizerTest();
    std::cout << "\n\nABSTRACT SYNTAX TREE EVALUATION TEST: \n\n" << std::endl;
    syntaxTreeTest();

    std::cout << "\n\nFULL EVALUATOR TEST, STRING -> TOKENS -> AST -> NUMBER\n\n"<<std::endl;

    AST::Environment env;
    env["x"] = 0.0;
    env["y"] = 1.0;
    fullTest("(x^2+32) * 10", env);
    fullTest("sin(x+y) * 10", env);

    env["x"] = PI;
    env["y"] = PI;
    fullTest("(x^2+32) * 10", env);
    fullTest("sin(x+y) * 10", env);
    fullTest("ln(cos(x + 2) * 3) * 3 + 23", env);
    fullTest("9*2^2*2",env);

    env["x"] = 2.0;
    fullTest("5(10x)^2 + 5x + 2",env);
    fullTest("3x + 2*3*3",env);

    return 0;
}



