#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <stdexcept>

namespace AST
{
using MathProcedure = std::function<double(const std::vector<double>&)>;


// object that holds all information about a mathematical operation in one unified place
struct Operation {
    enum class Format
    {
        INFIX = 0,
        PREFIX = 1,
        POSTFIX = 2,
        MULTIARG = 3,
    };

    MathProcedure procedure;
    int precedence = 0;
    Format format;
    Operation():format(Format::INFIX){}
    Operation(int precedence, const MathProcedure& procedure):precedence(precedence),procedure(procedure),format(Format::INFIX){}
    Operation(int precedence, const MathProcedure& procedure, Format format) :precedence(precedence), procedure(procedure), format(format){}

};  

class OperationRegistry {
private:
    std::unordered_map<std::string, Operation> registry;
public:
    OperationRegistry();
    // Lookup function to execute an operation
    double execute(const std::string& op, const std::vector<double>& args) const;

    const std::unordered_map<std::string, Operation>& getRegistry() const { return registry; }

};
};
