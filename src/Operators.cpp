#include "Operators.hpp"
#include "TreeNodes.h"
namespace AST
{
OperationRegistry::OperationRegistry()
{
    // binary operators
    B_Operation add;
    add.precedence = 1;
    add.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
            return args[0] + args[1];
        };
    add.identityfn = [](NodePtr op) -> NodePtr {
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return op;
        
        const auto* var_arg0 = dynamic_cast<const VariableNode*>(args[0].get());
        const auto* var_arg1 = dynamic_cast<const VariableNode*>(args[1].get());
        const auto* literal_arg0 = dynamic_cast<const NumberNode*>(args[0].get());
        const auto* literal_arg1 = dynamic_cast<const NumberNode*>(args[1].get());

        if(var_arg0 && var_arg1)
        {
            auto name0 = var_arg0->getName();
            auto name1 = var_arg1->getName();

            auto coeff0 = var_arg0->getCoeffecient();
            auto coeff1 = var_arg1->getCoeffecient();

            auto degree0 = var_arg0->getDegree();
            auto degree1 = var_arg1->getDegree();

            // like terms
            if(name0 == name1 && degree0 == degree1)
                if(coeff0+coeff1 == 0)
                    return std::make_shared<NumberNode>(0);
                else
                    return std::make_shared<VariableNode>(name0, coeff0+coeff1, degree0);
        }
        else if(literal_arg1 && literal_arg1->evaluate() == 0.0)
            return args[0];
        else if(literal_arg0 && literal_arg0->evaluate() == 0.0)
            return args[1];

        
        return op;
    };

    B_Operation sub;
    sub.precedence = 1;
    sub.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
            return args[0] - args[1];
        };
    B_Operation mul; 
    mul.precedence = 2;
    mul.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
            return args[0] * args[1];
        };

    mul.identityfn = [](NodePtr op) -> NodePtr {
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return op;
        
        const auto* var_arg0 = dynamic_cast<const VariableNode*>(args[0].get());
        const auto* var_arg1 = dynamic_cast<const VariableNode*>(args[1].get());
        const auto* literal_arg0 = dynamic_cast<const NumberNode*>(args[0].get());
        const auto* literal_arg1 = dynamic_cast<const NumberNode*>(args[1].get());

        if(var_arg0 && var_arg1)
        {
            auto name0 = var_arg0->getName();
            auto name1 = var_arg1->getName();

            auto coeff0 = var_arg0->getCoeffecient();
            auto coeff1 = var_arg1->getCoeffecient();

            auto degree0 = var_arg0->getDegree();
            auto degree1 = var_arg1->getDegree();

            // like terms
            if (name0 == name1)
            {
                auto coefficient = coeff0 * coeff1;
                auto degree = degree0 + degree1;

                if (degree == 0)
                    return std::make_shared<NumberNode>(coefficient);

                return std::make_shared<VariableNode>(
                    name0,
                    coefficient,
                    degree
                );
            }
        }
        else if(var_arg0 && literal_arg1)
        {
            return std::make_shared<VariableNode>(var_arg0->getName(), var_arg0->getCoeffecient() * literal_arg1->evaluate(), var_arg0->getDegree());
        }
        else if(var_arg1 && literal_arg0)
        {
            return std::make_shared<VariableNode>(var_arg1->getName(), var_arg1->getCoeffecient() * literal_arg0->evaluate(), var_arg1->getDegree());
        }
        else if(literal_arg1 && literal_arg1->evaluate() == 0.0 || literal_arg0 && literal_arg0->evaluate() == 0.0)
            return std::make_shared<NumberNode>(0);

        
        return op;
    };

    B_Operation pow;
    pow.precedence = 3;
    pow.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'^' requires 2 arguments");
            return std::pow(args[0], args[1]);
        };

    pow.identityfn = [](NodePtr op) -> NodePtr {
        const auto& args = op->getArguments();

        if (args.size() != 2)
            return op;

        const auto* base =
            dynamic_cast<const VariableNode*>(args[0].get());

        const auto* exponent =
            dynamic_cast<const NumberNode*>(args[1].get());

        if (exponent && exponent->evaluate() == 0.0)
            return std::make_shared<NumberNode>(1);

        if (base && exponent)
        {
            double n = exponent->evaluate();
            double coeff = std::pow(base->getCoeffecient(), exponent->evaluate());
            return std::make_shared<VariableNode>(
                base->getName(),
                coeff,
                base->getDegree() * n
            );
        }

        return op;
    };
        
    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::COMMUTATIVE));
    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));
    add.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));

    sub.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));

    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::COMMUTATIVE));
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::LEFT_ASSOCIATIVE));
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));

    pow.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::RIGHT_ASSOCIATIVE));

    registry["+"] = std::make_unique<B_Operation>(add);
    registry["-"] = std::make_unique<B_Operation>(sub);
    mul.properties.push_back(B_Operation::PropertyFlag(B_Operation::Property::DISTRIBUTIVE, {registry["+"].get(), registry["-"].get()}));
    registry["*"] = std::make_unique<B_Operation>(mul);
    registry["^"] = std::make_unique<B_Operation>(pow);


    // unary operators

    // unary minus, negates the value of its single argument unlike subtraction
    Operation unaryMinus;
    unaryMinus.precedence = 10;
    unaryMinus.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'-' requires 1 argument"); 
        return -args[0];
        };
    unaryMinus.format = Operation::Format::PREFIX;

    Operation sin;
    sin.precedence = 10;
    sin.procedure = [](const std::vector<double>& args) {
       if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        };
    sin.format = Operation::Format::PREFIX;
    
    Operation cos;
    cos.precedence = 10;
    cos.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument"); 
        return std::cos(args[0]);
        };
    cos.format = Operation::Format::PREFIX;

    

    Operation ln;
    ln.precedence = 10;
    ln.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'ln' requires 1 argument"); 
        return std::log(args[0]);
        };
    ln.format = Operation::Format::PREFIX;


    registry["unary-"] = std::make_unique<Operation>(unaryMinus);
    registry["sin"] = std::make_unique<Operation>(sin);
    registry["cos"] = std::make_unique<Operation>(cos);
    registry["ln"] = std::make_unique<Operation>(ln);
}

double OperationRegistry::execute(const std::string& opIdentifier, const std::vector<double>& args) const
{
    auto pair = registry.find(opIdentifier);
    if (pair == registry.end()) {
        throw std::runtime_error("Unknown operator: " + opIdentifier);
    }
    return pair->second->procedure(args);
}
NodePtr OperationRegistry::simplifyWithIdentities(NodePtr op) const
{

    const Operation* operation = getOperation(op->getName());
    if(!operation)
        return op;
    return operation->identityfn(op);
}

const Operation *OperationRegistry::getOperation(const std::string &op) const
{
    auto pair = registry.find(op);
    return pair != registry.end() ? pair->second.get() : nullptr;
}
bool B_Operation::hasProperty(B_Operation::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return true;
    }
    return false;   
}
const B_Operation::PropertyFlag *B_Operation::getProperty(B_Operation::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return &propFlag;
    }
    return nullptr;
}
};
