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
    MultiArg_OpDefinition add;
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

        if(phase == OptimizationPhase::COMPRESS && op->getCoeffecient() != 1.0 &&op->getDegree() == 1.0)
        {
            // distribute coeffecient of + operator into arguments
            auto clone = cloneSymbol(op,1,1);
            auto cloneOp = static_cast<OperatorNode*>(clone.get());
            auto args = cloneOp->getArguments();

            for (size_t i = 0; i < 2; ++i) {
                args[i] = Symbol::applySymbolData(args[i],op->getCoeffecient(),1);
            }
            cloneOp->setArguments(args);
            return clone;
        }
        else if (phase == OptimizationPhase::EXPAND && (op->getDegree() > 1.0 || op->getDegree() < -1.0))
        {
            double currentDegree = op->getDegree();
            bool isPositive = currentDegree > 0;
            
            // Determine the step direction
            double step = isPositive ? 1.0 : -1.0;
            double remainder = currentDegree - step;

            // Create a base copy of the expression raised to power 1 (or -1)
            // cloneSymbol(node, coefficient, degree)
            auto baseFactor = cloneSymbol(op, 1.0, step);

            // Create a copy of the expression holding the remaining fractional power
            auto remainderFactor = cloneSymbol(op, 1.0, remainder);

            // Multiply them together: (x+3)^1 * (x+3)^2.2
            auto newRoot = std::make_shared<OperatorNode>("*", std::vector<NodePtr>{baseFactor, remainderFactor}, op->getRegistry());

            // Re-apply the original outer coefficient
            return Symbol::applySymbolData(newRoot, op->getCoeffecient(), 1.0);
        }


        if(var_arg0 && var_arg1)
        {

            auto coeff0 = var_arg0->getCoeffecient();
            auto coeff1 = var_arg1->getCoeffecient();

            auto degree0 = var_arg0->getDegree();
            auto degree1 = var_arg1->getDegree();

            // like terms
            if(phase == OptimizationPhase::COMPRESS && var_arg0->equalBases(var_arg1) && degree0 == degree1)
            {
                if(coeff0+coeff1 == 0)
                    return std::make_shared<NumberNode>(0);
                else
                    return cloneSymbol(var_arg0,op->getCoeffecient()*(coeff0+coeff1),op->getDegree()*degree0);
            }
        }
        else if(args[1]->type == NodeType::Number && static_cast<const NumberNode*>(args[1].get())->evaluate() == 0.0)
            return Symbol::applySymbolData(args[0],op->getCoeffecient(),op->getDegree());
        else if(args[0]->type == NodeType::Number && static_cast<const NumberNode*>(args[0].get())->evaluate() == 0.0)
            return Symbol::applySymbolData(args[1],op->getCoeffecient(),op->getDegree());

        
        return node;
    };

    MultiArg_OpDefinition sub;
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
            args[1] = Symbol::applySymbolData(args[1],-1,1);
        }

        op->setArguments(args);
        return nodeClone;

    };
    MultiArg_OpDefinition mul; 
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
        
        auto var_arg0 = args[0]->asSymbol();
        auto var_arg1 = args[1]->asSymbol();        

        if(var_arg0 && var_arg1&& (phase == OptimizationPhase::COMPRESS || (var_arg0->type == NodeType::Variable && var_arg1->type==NodeType::Variable)))
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
            // coeffecient crushing
            double num_val = static_cast<const NumberNode*>(args[1].get())->evaluate();
            auto safe_clone = args[0]->clone();
            auto symbol = safe_clone->asSymbol();
            symbol->setCoeffecient(symbol->getCoeffecient() * num_val);
            return safe_clone;
        }
        else if(var_arg1 && args[0]->type == NodeType::Number)
        {
            // coeffecient crushing
            double num_val = static_cast<const NumberNode*>(args[0].get())->evaluate();
            auto safe_clone = args[1]->clone();
            auto symbol = safe_clone->asSymbol();
            symbol->setCoeffecient(symbol->getCoeffecient() * num_val);
            return safe_clone;
        }
        else if(args[0]->type == NodeType::Number && static_cast<const NumberNode*>(args[0].get())->evaluate() == 0.0 || args[1]->type == NodeType::Number && static_cast<const NumberNode*>(args[1].get())->evaluate() == 0.0)
            return std::make_shared<NumberNode>(0);
        if(phase == OptimizationPhase::EXPAND && var_arg0 && var_arg1)
        {
            OperatorNode* op_0 = static_cast<OperatorNode*>(var_arg0);
            OperatorNode* op_1 = static_cast<OperatorNode*>(var_arg1);
            
            std::vector<NodePtr> args0 = op_0->getArguments();
            std::vector<NodePtr> args1 = op_1->getArguments();
            // monomial expansions
            if(var_arg0->getName() == "+" && var_arg0->getDegree() == 1)
            {

                auto newArgs0 = std::make_shared<OperatorNode>("*",std::vector<NodePtr>{args0[0],args[1]},op->getRegistry());
                auto newArgs1 = std::make_shared<OperatorNode>("*",std::vector<NodePtr>{args0[1],args[1]},op->getRegistry());

                auto newRoot = std::make_shared<OperatorNode>("+",std::vector<NodePtr>{newArgs0,newArgs1},op->getRegistry());
                
                return Symbol::applySymbolData(newRoot,op->getCoeffecient(),op->getDegree());

            }
            else if(var_arg1->getName() == "+" && var_arg1->getDegree() == 1)
            {
                auto newArgs0 = std::make_shared<OperatorNode>("*",std::vector<NodePtr>{args[0],args1[0]},op->getRegistry());
                auto newArgs1 = std::make_shared<OperatorNode>("*",std::vector<NodePtr>{args[0],args1[1]},op->getRegistry());

                auto newRoot = std::make_shared<OperatorNode>("+",std::vector<NodePtr>{newArgs0,newArgs1},op->getRegistry());
                
                return Symbol::applySymbolData(newRoot,op->getCoeffecient(),op->getDegree());
            }
        }

        
        return node;
    };

    MultiArg_OpDefinition div;
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

    MultiArg_OpDefinition pow;
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
                    
                    // nested power node
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
                    // wrap structurally
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
        
    add.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::COMMUTATIVE));
    add.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_LEFT_ASSOCIATIVE));
    add.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_RIGHT_ASSOCIATIVE));

    sub.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_LEFT_ASSOCIATIVE));

    mul.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::COMMUTATIVE));
    mul.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_LEFT_ASSOCIATIVE));
    mul.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_RIGHT_ASSOCIATIVE));

    div.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_LEFT_ASSOCIATIVE));

    pow.properties.push_back(MultiArg_OpDefinition::PropertyFlag(MultiArg_OpDefinition::Property::BINARY_RIGHT_ASSOCIATIVE));

    registry["+"] = std::make_unique<MultiArg_OpDefinition>(add);
    registry["-"] = std::make_unique<MultiArg_OpDefinition>(sub);
    registry["*"] = std::make_unique<MultiArg_OpDefinition>(mul);
    registry["/"] = std::make_unique<MultiArg_OpDefinition>(div);
    registry["^"] = std::make_unique<MultiArg_OpDefinition>(pow);

    registry["[implicit*]"] = std::make_unique<MultiArg_OpDefinition>(mul);
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


    // n-ary functions

    MultiArg_OpDefinition max;
    max.precedence = 10;
    max.format = OpDefiniton::Format::EXPLICIT_FUNCTION;
    max.procedure = [](const std::vector<double>& args)
    {
        if(args.empty())
            throw std::runtime_error("Max requires at least 1 argument!");
        double max = args[0];
        for(size_t i=1;i<args.size();i++)
        {
            if(args[i]>args[0])
                max = args[i];
        }
        return max;
    };
    max.identityfn = [](NodePtr node, auto phase) -> NodePtr
    {
        if (node->type != NodeType::Operator) return node;
        const OperatorNode* op = static_cast<const OperatorNode*>(node.get());
        
        auto originalArgs = op->getArguments();
        std::vector<NodePtr> flattenedArgs;
        flattenedArgs.reserve(originalArgs.size() * 2);

        bool changed = false;
        for (const auto& arg : originalArgs)
        {
            if (arg->type == NodeType::Operator && arg->getName() == op->getName())
            {
                const OperatorNode* argOp = static_cast<const OperatorNode*>(arg.get());
                auto innerArgs = argOp->getArguments();
                flattenedArgs.insert(flattenedArgs.end(), innerArgs.begin(), innerArgs.end());
                changed = true;
            }
            else
            {
                flattenedArgs.push_back(arg);
            }
        }
        if (changed)
        {
            auto clone = op->clone();
            static_cast<OperatorNode*>(clone.get())->setArguments(flattenedArgs);
            return clone;
        }

    return node;
};

    max.properties.push_back(AST::MultiArg_OpDefinition::Property::COMMUTATIVE);
    
    MultiArg_OpDefinition min;
    min.precedence = 10;
    min.format = OpDefiniton::Format::EXPLICIT_FUNCTION;
    min.procedure = [](const std::vector<double>& args)
    {
        if(args.empty())
            throw std::runtime_error("min requires at least 1 argument!");
        double min = args[0];
        for(size_t i=1;i<args.size();i++)
        {
            if(args[i]<args[0])
                min = args[i];
        }
        return min;
    };
    min.identityfn = max.identityfn;
    min.properties = max.properties;


    registry["max"] = std::make_unique<MultiArg_OpDefinition>(max);
    registry["min"] = std::make_unique<MultiArg_OpDefinition>(min);
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
bool MultiArg_OpDefinition::hasProperty(MultiArg_OpDefinition::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return true;
    }
    return false;   
}
const MultiArg_OpDefinition::PropertyFlag *MultiArg_OpDefinition::getProperty(MultiArg_OpDefinition::Property property) const
{
    for (const auto& propFlag : properties)
    {
        if (propFlag.property == property)
            return &propFlag;
    }
    return nullptr;
}
};
