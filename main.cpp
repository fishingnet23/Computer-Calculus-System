#include <iostream>
#include "TreeNodes.h"
#include "Tokenizer.hpp"

static void testToken(std::string src)
{


    Tokenizer t(src);
    std::cout << src << std::endl;
    auto res1 = t.tokenize();
    std::cout << "TOKENIZED: (NOT PARSED, JUST TURNED FROM TEXT INTO TOKENS)" << std::endl;
    for (const auto& word : res1)
        std::cout << word << " , ";
    std::cout << "\n"<<std::endl;
}

static void tokenizerTest()
{

    testToken("sin(x) + 5 + 3 + 2");
    testToken("ln(cos(x + 2) * 3) * 3 + 23");
    testToken("\n x^2 / 23 * 100 + sin(x*x*x*x)\n + 532");

}


static void syntaxTreeTest()
{
    OperationRegistry registry;

    NodePtr xNode = std::make_shared<VariableNode>("x");

    NodePtr sinX = std::make_shared<OperatorNode>("sin", std::vector<NodePtr>{xNode}, registry);
    NodePtr cosX = std::make_shared<OperatorNode>("cos", std::vector<NodePtr>{xNode}, registry);
    NodePtr lnX = std::make_shared<OperatorNode>("ln", std::vector<NodePtr>{xNode}, registry);


    NodePtr totalExpr = std::make_shared<OperatorNode>("+", std::vector<NodePtr>{sinX, cosX}, registry);

    std::cout << "Expression: " << totalExpr->toString() << "\n";

    Environment env;
    env["x"] = 0.0;
    std::cout << "Result when x=0: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 1.0;
    std::cout << "Result when x=1: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 2.71828182845904523536;
    std::cout << "Result when x=e: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 3.14159265358979323846;
    std::cout << "Result when x=PI: " << totalExpr->evaluate(env) << "\n";
}

int main() {
    std::cout << "\n\nTOKENIZER TEST: \n\n" << std::endl;
    tokenizerTest();
    std::cout << "\n\nABSTRACT SYNTAX TREE EVALUATION TEST: \n\n" << std::endl;
    syntaxTreeTest();

    return 0;
}



