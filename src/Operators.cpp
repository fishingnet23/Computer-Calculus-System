#include "Operators.hpp"
#include "TreeNodes.h"
namespace AST
{

static NodePtr cloneSymbol(const Symbol* var, double coeffecient, double degree)
{
    auto clone = var->clone();
    std::cout<<coeffecient<<"  "<<degree<<"\n";
    clone->asSymbol()->setCoeffecient(coeffecient);
    clone->asSymbol()->setDegree(degree);
    return clone;
}

OperationRegistry::OperationRegistry()
{
    // binary operators
    B_OpDefinition add;
    add.precedence = 1;
    add.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'+' requires 2 arguments");
            return args[0] + args[1];
        };
    add.identityfn = [](NodePtr node) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;
        
        const auto* var_arg0 = args[0]->asSymbol();
        const auto* var_arg1 = args[1]->asSymbol();


        if(var_arg0 && var_arg1)
        {

            auto coeff0 = var_arg0->getCoeffecient();
            auto coeff1 = var_arg1->getCoeffecient();

            auto degree0 = var_arg0->getDegree();
            auto degree1 = var_arg1->getDegree();

            // like terms
            if(var_arg0->equalBases(var_arg1) && degree0 == degree1)
                if(coeff0+coeff1 == 0)
                    return std::make_shared<NumberNode>(0);
                else
                    return cloneSymbol(var_arg0,coeff0+coeff1,degree0);
        }
        else if(args[1]->type == NodeType::Number && static_cast<const NumberNode*>(args[1].get())->evaluate() == 0.0)
            return args[0];
        else if(args[0]->type == NodeType::Number && static_cast<const NumberNode*>(args[0].get())->evaluate() == 0.0)
            return args[1];

        
        return node;
    };

    B_OpDefinition sub;
    sub.precedence = 1;
    sub.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'-' requires 2 arguments");
            return args[0] - args[1];
        };
    B_OpDefinition mul; 
    mul.precedence = 2;
    mul.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
            return args[0] * args[1];
        };

    mul.identityfn = [](NodePtr node) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;
        
        const auto* var_arg0 = args[0]->asSymbol();
        const auto* var_arg1 = args[1]->asSymbol();


        if(var_arg0 && var_arg1)
        {

            auto coeff0 = var_arg0->getCoeffecient();
            auto coeff1 = var_arg1->getCoeffecient();

            auto degree0 = var_arg0->getDegree();
            auto degree1 = var_arg1->getDegree();

            // like terms
            if (var_arg0->equalBases(var_arg1))
            {
                auto coefficient = coeff0 * coeff1;
                auto degree = degree0 + degree1;

                if (degree == 0)
                    return std::make_shared<NumberNode>(coefficient);
                return cloneSymbol(var_arg0,coefficient,degree);
            }
        }
        else if(var_arg0 && args[1]->type == NodeType::Number)
        {
            return cloneSymbol(var_arg0, var_arg0->getCoeffecient() * (static_cast<const NumberNode*>(args[1].get()))->evaluate(), var_arg0->getDegree());
        }
        else if(var_arg1 && args[0]->type == NodeType::Number)
        {
            return cloneSymbol(var_arg1, var_arg1->getCoeffecient() * (static_cast<const NumberNode*>(args[0].get()))->evaluate(), var_arg1->getDegree());
        }
        else if(args[0]->type == NodeType::Number && static_cast<const NumberNode*>(args[0].get())->evaluate() == 0.0 || args[1]->type == NodeType::Number && static_cast<const NumberNode*>(args[1].get())->evaluate() == 0.0)
            return std::make_shared<NumberNode>(0);

        
        return node;
    };

    B_OpDefinition pow;
    pow.precedence = 3;
    pow.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'^' requires 2 arguments");
            return std::pow(args[0], args[1]);
        };

    pow.identityfn = [](NodePtr node) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;

        const auto* base = args[0]->asSymbol();
        
        if(args[1]->type != NodeType::Number)
            return node;
        const auto* exponent = static_cast<const NumberNode*>(args[1].get());

        if (exponent->evaluate() == 0.0)
            return std::make_shared<NumberNode>(1);

        if (base && exponent)
        {
            return cloneSymbol(base, std::pow(base->getCoeffecient(), exponent->evaluate()), base->getDegree() * exponent->evaluate());
        }

        return node;
    };
        
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::COMMUTATIVE));
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    sub.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));

    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::COMMUTATIVE));
    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));
    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    pow.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    registry["+"] = std::make_unique<B_OpDefinition>(add);
    registry["-"] = std::make_unique<B_OpDefinition>(sub);
    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::DISTRIBUTIVE, {registry["+"].get(), registry["-"].get()}));
    registry["*"] = std::make_unique<B_OpDefinition>(mul);
    registry["^"] = std::make_unique<B_OpDefinition>(pow);


    // unary operators

    // unary minus, negates the value of its single argument unlike subtraction
    OpDefiniton unaryMinus;
    unaryMinus.precedence = 10;
    unaryMinus.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'-' requires 1 argument"); 
        return -args[0];
        };
    unaryMinus.format = OpDefiniton::Format::PREFIX;

    OpDefiniton sin;
    sin.precedence = 10;
    sin.procedure = [](const std::vector<double>& args) {
       if (args.size() != 1) throw std::runtime_error("'sin' requires 1 argument");
        return std::sin(args[0]);
        };
    sin.format = OpDefiniton::Format::PREFIX;
    
    OpDefiniton cos;
    cos.precedence = 10;
    cos.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'cos' requires 1 argument"); 
        return std::cos(args[0]);
        };
    cos.format = OpDefiniton::Format::PREFIX;

    

    OpDefiniton ln;
    ln.precedence = 10;
    ln.procedure = [](const std::vector<double>& args) {
        if (args.size() != 1) throw std::runtime_error("'ln' requires 1 argument"); 
        return std::log(args[0]);
        };
    ln.format = OpDefiniton::Format::PREFIX;


    registry["unary-"] = std::make_unique<OpDefiniton>(unaryMinus);
    registry["sin"] = std::make_unique<OpDefiniton>(sin);
    registry["cos"] = std::make_unique<OpDefiniton>(cos);
    registry["ln"] = std::make_unique<OpDefiniton>(ln);
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

    const OpDefiniton* operation = getOperation(op->getName());
    if(!operation)
        return op;
    return operation->identityfn(op);
}

const OpDefiniton *OperationRegistry::getOperation(const std::string &op) const
{
    auto pair = registry.find(op);
    return pair != registry.end() ? pair->second.get() : nullptr;
}
OpDefiniton::Format OperationRegistry::getOperationFormat(const std::string &op) const
{
    auto opObj = getOperation(op);
    if(!opObj)
        return OpDefiniton::Format::NONE;
    return opObj->format;
}
bool B_OpDefinition::hasProperty(B_OpDefinition::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return true;
    }
    return false;   
}
const B_OpDefinition::PropertyFlag *B_OpDefinition::getProperty(B_OpDefinition::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return &propFlag;
    }
    return nullptr;
}
};
