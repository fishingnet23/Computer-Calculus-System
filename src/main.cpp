#include <iostream>
#include "AST.hpp"
#include <sstream>

constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;

void clearConsole() {
    std::cout << "\033[2J\033[1;1H"; 
}

// good test cases
// 7/3*x^5 - 11*x^4 + 1/2*x^5 + 13*x^2 - 4/5*x^3 + 9*x^4 - 22/7*x + 15*x^2 - 6*x^5 + 1/8*x^3 - 14*x + 19
// (x^2 - x^2)ln(x) + 3x^2(x^0 + 4) - 15x^2
// 1 / 2x / 3x^2 / (1 / 4x)
// (2x^2)^3^2 - 512x^18
// sin(x) / cos(x) -x ^ 0 * sin(x) / cos(x)
// x - -2x^2 - (3x^2 - -x)


int main() {
    
    AST::OperationRegistry registry;
    
    do
    {
    std::cout<<"Enter a math expression in calculator language (example: 5x^2 + cos(x) + 5) : "<<std::endl;
    std::string expression;
    std::getline(std::cin, expression);
    std::cout <<"raw input: " << expression<<std::endl;
    AST::Tree tree(expression,registry);
    try
    {
        tree.simplify();
    }
    catch(const std::runtime_error& e)
    {
        e.what();
        continue;
    }
    std::cout << tree.toString() << std::endl;

    std::cout << "Enter variables and values seperated by commas: " << std::endl;
    std::string vars;
    std::getline(std::cin, vars);

    std::vector<std::string> tokens;
    std::stringstream ss(vars);
    std::string token;

    // Split by the comma delimiter
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    
    AST::Environment env(tokens);
    env.print();

    std::cout << "Evaluation: " << tree.evaluate(env)<<std::endl;

    std::cout << "Exit? (1 for yes) ";
    int exit;

    std::cin >> exit;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "\n\n\n" << std::endl;
    if(exit == 1)
        break;
    }while(true);

    std::cout << "Thank you for testing the program!" << std::endl;

    return 0;
}



