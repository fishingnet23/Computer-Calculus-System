#include <iostream>
#include "TreeNodes.h"
#include "Tokenizer.hpp"

static int finalProgram()
{
    // Instantiate our unified registry
    OperationRegistry registry;

    NodePtr xNode = std::make_shared<VariableNode>("x");

    NodePtr sinX = std::make_shared<OperatorNode>("sin", std::vector<NodePtr>{xNode}, registry);
    NodePtr cosX = std::make_shared<OperatorNode>("cos", std::vector<NodePtr>{xNode}, registry);
    NodePtr lnX = std::make_shared<OperatorNode>("ln", std::vector<NodePtr>{xNode}, registry);


    NodePtr totalExpr = std::make_shared<OperatorNode>("+", std::vector<NodePtr>{sinX, cosX}, registry);

    // Print the symbolic structure
    std::cout << "Expression: " << totalExpr->toString() << "\n";

    Environment env;
    env["x"] = 0.0;
    std::cout << "Result when x=0: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 1.0;
    std::cout << "Result when x=1: " << totalExpr->evaluate(env) << "\n";
    env["x"] = 2.71828182845904523536;
    std::cout << "Result when x=e: " << totalExpr->evaluate(env) << "\n";

    return 0;
}

void testToken(std::string token, const OperationRegistry& registry)
{


    Tokenizer t(token,registry);
    std::cout << token << std::endl;
    auto res1 = t.tokenize();

    for (const auto& word : res1)
        std::cout << word << " , ";
    std::cout << "\n"<<std::endl;
}

int tokenizerTest()
{
    OperationRegistry registry;

    std::string test = "sin(x) + 5 + 3 + 2";
    std::string test2 = "sin(cos(x + 2) * 3) * 3 + 23";

    testToken(test,registry);
    testToken(test2,registry);
    testToken("\n x^2 / 23 * 100 + sin(x*x*x*x)\n + 532", registry);

    return 0;
}


void main() {
    //return finalProgram();
    tokenizerTest();
}



