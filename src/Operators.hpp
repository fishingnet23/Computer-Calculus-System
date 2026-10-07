#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cmath>
#include <stdexcept>
#include "Node.hpp"

namespace AST
{



// object that holds all information about a mathematical operation in one unified place
struct Operation {

    virtual ~Operation() = default;

    using MathProcedure = std::function<double(const std::vector<double>&)>;
    using SimplificationRule = std::function<NodePtr(const std::vector<NodePtr>&)>;

    
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
struct B_Operation : public Operation
{

    enum class Property
    {
        LEFT_ASSOCIATIVE = 1,
        RIGHT_ASSOCIATIVE = 2,
        COMMUTATIVE = 4,
        DISTRIBUTIVE = 8,
        
    };
    struct PropertyFlag
    {
        Property property;
        std::vector<Operation*> applicableOperations;
        PropertyFlag(Property prop) : property(prop) {applicableOperations = {};}
        PropertyFlag(Property prop, const std::vector<Operation*>& applicableOps) : property(prop), applicableOperations(applicableOps) {}
        void addOperation(Operation* op) { applicableOperations.push_back(op); }
        operator Property() const { return property; }
    };

    std::vector<PropertyFlag> properties;

    B_Operation(int precedence, const Operation::MathProcedure& procedure, Format format = Format::INFIX)
        : Operation(precedence, procedure, format) {properties = {};}
    B_Operation() : Operation() {properties = {};}
    B_Operation(const Operation& op) : Operation(op) {properties = {};}
    std::vector<PropertyFlag>&  getProperties() { return properties; }
    bool hasProperty(B_Operation::Property property) const;

};

class OperationRegistry {
private:
    std::unordered_map<std::string, std::unique_ptr<Operation>> registry;
public:
    OperationRegistry();
    // Lookup function to execute an operation
    double execute(const std::string& op, const std::vector<double>& args) const;

    const Operation* getOperation(const std::string& op) const;

    const std::unordered_map<std::string, std::unique_ptr<Operation>>& getRegistry() const { return registry; }

};
};
