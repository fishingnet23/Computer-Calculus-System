#include "Operators.hpp"
#include "TreeNodes.h"
namespace AST
{

static NodePtr cloneSymbol(const Symbol* var, double coeffecient, double degree)
{
    auto clone = var->clone();
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
    add.identityfn = [](NodePtr node, auto phase) -> NodePtr {
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
            if(phase == OptimizationPhase::COMPRESS && var_arg0->equalBases(var_arg1) && degree0 == degree1)
                if(coeff0+coeff1 == 0)
                    return std::make_shared<NumberNode>(0);
                else
                    return cloneSymbol(var_arg0,op->getCoeffecient()*(coeff0+coeff1),op->getDegree()*degree0);
            else if((op->getCoeffecient()<0||phase == OptimizationPhase::EXPAND) && op->getDegree() == 1.0)
            {
                // distribute coeffecient of + operator into arguments
                auto clone = cloneSymbol(op,1,1);
                auto cloneOp = static_cast<OperatorNode*>(clone.get());
                auto args = cloneOp->getArguments();

                for (size_t i = 0; i < 2; ++i) {
                    double newCoeff = (i == 0 ? coeff0 : coeff1) * op->getCoeffecient();
                    
                    if (args[i]->type == NodeType::Operator) {
                        // If it's a compound operator, push the weight structurally via multiplication
                        auto coeffNode = std::make_shared<NumberNode>(newCoeff);
                        args[i] = std::make_shared<OperatorNode>("*", std::vector<NodePtr>{coeffNode, args[i]}, op->getRegistry());
                    } else {
                        // Safe to directly stamp onto an atomic variable symbol leaf
                        args[i]->asSymbol()->setCoeffecient(newCoeff);
                    }
                    
                }
                cloneOp->setArguments(args);

                return clone;
            }
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
    sub.identityfn = [](NodePtr node, auto phase) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        OperatorNode* originalOperator = static_cast<OperatorNode*>(node.get());

        if(originalOperator->getArguments().size()!= 2)
            return node;
        

        auto nodeClone = node->clone();
        OperatorNode* op = static_cast<OperatorNode*>(nodeClone.get());
        op->setName("+");
        auto args = op->getArguments();

        auto var_arg1 = args[1]->asSymbol();
        
        if(var_arg1)
        {
            if (args[1]->type == NodeType::Operator)
            {
                auto negOne = std::make_shared<NumberNode>(-1.0);
                args[1] = std::make_shared<OperatorNode>("*", std::vector<NodePtr>{negOne,args[1]},op->getRegistry());
            }
            else
            {
                var_arg1->negate();
            }
        }
        else if(args[1]->type == NodeType::Number)
        {
            args[1] = std::make_shared<NumberNode>(-static_cast<const NumberNode*>(args[1].get())->evaluate());
        }

        op->setArguments(args);
        return nodeClone;

    };
    B_OpDefinition mul; 
    mul.precedence = 2;
    mul.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
            return args[0] * args[1];
        };
    mul.identityfn = [](NodePtr node, auto phase) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;
        
        const auto var_arg0 = args[0]->asSymbol();
        const auto* var_arg1 = args[1]->asSymbol();
        
        if(phase == OptimizationPhase::COMPRESS)
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
                return cloneSymbol(var_arg0,op->getCoeffecient()*coefficient,op->getDegree()*degree);
            }
            else
            {
                // save unneeded allocation
                if(coeff0 == 1.0 && coeff1 == 1.0)
                    return node;
                if (args[0]->type == NodeType::Operator || args[1]->type == NodeType::Operator) {
                    return node; 
                }

                double nestedCoeffecient = coeff0*coeff1;
                auto newArg0 = cloneSymbol(var_arg0,1,degree0);
                auto newArg1 = cloneSymbol(var_arg1,1,degree1);
                auto newNode = node->clone();
                auto newOp = static_cast<OperatorNode*>(newNode.get());
                newOp->setArguments({newArg0,newArg1});
                newOp->setCoeffecient(newOp->getCoeffecient()*nestedCoeffecient);

                return newNode;
            }
        }
        else if(var_arg0 && args[1]->type == NodeType::Number)
        {    
            double num_val = static_cast<const NumberNode*>(args[1].get())->evaluate();
            auto safe_clone = args[0]->clone();
            auto symbol = safe_clone->asSymbol();
            symbol->setCoeffecient(symbol->getCoeffecient() * num_val);
            return safe_clone;
        }
        else if(var_arg1 && args[0]->type == NodeType::Number)
        {
            double num_val = static_cast<const NumberNode*>(args[0].get())->evaluate();
            auto safe_clone = args[1]->clone();
            auto symbol = safe_clone->asSymbol();
            symbol->setCoeffecient(symbol->getCoeffecient() * num_val);
            return safe_clone;
        }
        else if(args[0]->type == NodeType::Number && static_cast<const NumberNode*>(args[0].get())->evaluate() == 0.0 || args[1]->type == NodeType::Number && static_cast<const NumberNode*>(args[1].get())->evaluate() == 0.0)
            return std::make_shared<NumberNode>(0);

        
        return node;
    };

    B_OpDefinition div;
    div.precedence = 2;
    div.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'*' requires 2 arguments");
        if(args[1] == 0) throw std::runtime_error("Division by zero error!");
        return args[0] / args[1];
        };
    div.identityfn = [](NodePtr node,auto phase) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        OperatorNode* originalOperator = static_cast<OperatorNode*>(node.get());

        if(originalOperator->getArguments().size()!= 2)
            return node;
        

        auto nodeClone = node->clone();
        OperatorNode* op = static_cast<OperatorNode*>(nodeClone.get());
        op->setName("*");
        auto args = op->getArguments();

        auto var_arg1 = args[1]->asSymbol();
        
        if (var_arg1)
        {
            if (args[1]->type == NodeType::Operator)
            {
                auto negOne = std::make_shared<NumberNode>(-1.0);
                args[1] = std::make_shared<OperatorNode>("^", std::vector<NodePtr>{args[1], negOne},op->getRegistry());
            }
            else
            {
                var_arg1->inverse();
            }
        }
        else if(args[1]->type == NodeType::Number)
        {
            double val = static_cast<const NumberNode*>(args[1].get())->evaluate();

            if(val == 0)
                throw std::runtime_error("Division by zero error!");
            args[1] = std::make_shared<NumberNode>(1/val);
        }

        op->setArguments(args);

        return nodeClone;
        
        
        
    };

    B_OpDefinition pow;
    pow.precedence = 4;
    pow.procedure = [](const std::vector<double>& args) {
        if (args.size() != 2) throw std::runtime_error("'^' requires 2 arguments");
            return std::pow(args[0], args[1]);
        };

    pow.identityfn = [](NodePtr node, auto phase) -> NodePtr {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;

        const auto* base = args[0]->asSymbol();
        
        if(args[1]->type != NodeType::Number)
            return node;
        const auto* exponent = static_cast<const NumberNode*>(args[1].get());
        double exp_val = exponent->evaluate();

        if (exp_val == 0.0)
            return std::make_shared<NumberNode>(std::pow(op->getCoeffecient(),op->getDegree()));

        if (!base)
        {  
            return node;
        }
        auto baseOpName = static_cast<const OperatorNode*>(args[0].get())->getName();
        if (args[0]->type == NodeType::Operator && (baseOpName == "*"))
        {
            auto mulClone = args[0]->clone();
            auto mulOp = static_cast<OperatorNode*>(mulClone.get()); 

            auto mulArgs = mulOp->getArguments();
            for (size_t i = 0; i < 2; ++i) {
                if (mulArgs[i]->type == NodeType::Operator)
                {
                    auto* subOp = static_cast<OperatorNode*>(mulArgs[i].get());
                    
                    // Case A: It's a nested power node -> (x^m)^n. Multiply exponents structurally!
                    if (subOp->getName() == "^")
                    {
                        auto subArgs = subOp->getArguments();
                        if (subArgs[1]->type == NodeType::Number)
                        {
                            double inner_exp = static_cast<const NumberNode*>(subArgs[1].get())->evaluate();
                            subArgs[1] = std::make_shared<NumberNode>(inner_exp * exp_val);
                            subOp->setArguments(subArgs);
                        }
                    }
                    // Case B: It's any other operator -> wrap it structurally so it can unroll later
                    else
                    {
                        mulArgs[i] = std::make_shared<OperatorNode>("^", std::vector<NodePtr>{mulArgs[i], std::make_shared<NumberNode>(exp_val)}, op->getRegistry());
                    }
                }
                else if (auto symbol = mulArgs[i]->asSymbol())
                {
                    // Safe to treat as a true leaf variable/symbol leaf node
                    mulArgs[i] = cloneSymbol(symbol, std::pow(symbol->getCoeffecient(), exp_val), symbol->getDegree() * exp_val);
                }
                else if (mulArgs[i]->type == NodeType::Number)
                {
                    mulArgs[i] = std::make_shared<NumberNode>(std::pow(static_cast<const NumberNode*>(mulArgs[i].get())->evaluate(), exp_val));
                }

            }

            mulOp->setArguments(mulArgs);

            return mulClone;
            
        }
        return cloneSymbol(base, std::pow(base->getCoeffecient(), exp_val), base->getDegree() * exp_val);
        

    };
        
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::COMMUTATIVE));
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));
    add.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    sub.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));

    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::COMMUTATIVE));
    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));
    mul.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    div.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::LEFT_ASSOCIATIVE));

    pow.properties.push_back(B_OpDefinition::PropertyFlag(B_OpDefinition::Property::RIGHT_ASSOCIATIVE));

    registry["+"] = std::make_unique<B_OpDefinition>(add);
    registry["-"] = std::make_unique<B_OpDefinition>(sub);
    registry["*"] = std::make_unique<B_OpDefinition>(mul);
    registry["/"] = std::make_unique<B_OpDefinition>(div);
    registry["^"] = std::make_unique<B_OpDefinition>(pow);

    registry["[implicit*]"] = std::make_unique<B_OpDefinition>(mul);
    registry["[implicit*]"]->identityfn = [](NodePtr node, auto phase)->NodePtr
    {
        if(node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        const auto& args = op->getArguments();
        if(args.size()!= 2)
            return node;

        auto clone = node->clone();
        static_cast<OperatorNode*>(clone.get())->setName("*");
        return clone;
    };
    registry["[implicit*]"]->precedence++;

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
NodePtr OperationRegistry::simplifyWithIdentities(NodePtr op, OptimizationPhase phase) const
{

    const OpDefiniton* operation = getOperation(op->getName());
    if(!operation)
        return op;
    return operation->identityfn(op,phase);
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
