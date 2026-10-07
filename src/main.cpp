#include <iostream>
#include "AST.hpp"
#include <sstream>

constexpr double PI = 3.14159265358979323846;
constexpr double E = 2.71828182845904523536;



int main() {
    AST::OperationRegistry registry;
    
    std::cout<<"Enter an equation in calculator syntax : "<<std::endl;
    std::string equation;
    std::getline(std::cin, equation);
    AST::Tree tree(equation,registry);
    tree.simplify();
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


    return 0;
}



