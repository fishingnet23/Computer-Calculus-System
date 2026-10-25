#include <iostream>
#include "AST/AlgebraSyntaxTree.hpp"
#include <sstream>


// good test cases
// 7/3*x^5 - 11*x^4 + 1/2*x^5 + 13*x^2 - 4/5*x^3 + 9*x^4 - 22/7*x + 15*x^2 - 6*x^5 + 1/8*x^3 - 14*x + 19
// (x^2 - x^2) + 3x^2(x^0 + 4) - 15x^2
// 1 / 2x / 3x^2 / (1 / 4x)
// (2x^2)^3^2 - 512x^18
// sin(x) / cos(x) -x ^ 0 * sin(x) / cos(x)
// x - -2x^2 - (3x^2 - -x)


AST::AlgebraExpression promptUserForExpression()
{
    std::cout<<"Enter a math expression in calculator language (example: 5x^2 + cos(x) + 5) : "<<std::endl;
    std::string expression;
    std::getline(std::cin, expression);
    std::cout <<"raw input: " << expression<<std::endl;
    AST::AlgebraExpression tree;
    try
    {   
        tree.buildFrom(expression);
        std::cout << "raw AST: " << tree.postfixNotation() << std::endl;
        tree.cannonize();
    }
    catch(const std::exception& e)
    {
        std::cout << "Error when building AST from text: " << e.what() << std::endl;
        return AST::AlgebraExpression("0"); // default
    }

    return tree;
}

AST::Environment promptUserForEnvironment()
{

    
    
}

void keepEvaluating(const AST::AlgebraExpression& tree)
{    
    std::cout << "Expression to evaluate: " << tree.toString() << std::endl;
    std::cout << "Enter variables and values seperated by commas (or 'exit' to exit evaluator): " << std::endl;
    do
    {
        std::string vars;
        std::getline(std::cin, vars);
        if(vars == "exit")
            break;
        
        std::vector<std::string> tokens;
        std::stringstream ss(vars);
        std::string token;

        

        // Split by the comma delimiter
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        AST::Environment env(tokens);
        std::cout << "Parsed environment: ";
        env.print();
        try
        {
            std::cout << "Evaluation: " << tree.evaluate(env)<<std::endl;
        }
        catch(const std::exception& e)
        {
            std::cout << "Error when evaluating AST with the environment given: " << e.what() << std::endl;
        }
    }while(true);
}

int promptForOption()
{
    
    std::cout << "\n\n";
    std::cout << "What would you like to do? " << std::endl;
    std::cout << "0 - Exit" << std::endl;
    std::cout << "1 - Enter a new expression" << std::endl;
    std::cout << "2 - Evaluate the current expression" << std::endl;
    std::cout << "Enter your choice: ";
    int option;

    std::cin >> option;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\n"<<std::endl;
    return option;
}

int main() 
{
    AST::AlgebraExpression tree;
    do
    {
        int option = promptForOption();
        if(option == 0)
            break;
        else if(option == 1)
        {
            tree = promptUserForExpression();
            std::cout <<"FINAL OUTPUT IN POSTFIX: " <<tree.postfixNotation() << std::endl;
            std::cout <<"FINAL OUTPUT: " << tree.toString() << std::endl;
        }
        else if(option == 2)
        {
            keepEvaluating(tree);
        }
    }
    while(true);

    std::cout << "Thank you for testing the program!" << std::endl;

    return 0;
}



