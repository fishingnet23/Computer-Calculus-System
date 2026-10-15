#include "Nodes.hpp"
#include "OperatorDefinitions.hpp"

namespace AST
{
    
    int Node::nodeRank(const std::shared_ptr<Node> &node)
    {
        if (node->type() == NodeType::Number)
            return 0;

        if (node->type() == NodeType::Variable)
            return 1;

        if (node->type() == NodeType::Operator)
            return 2;

        return 3;
    }
    bool Node::canonicalLess(const std::shared_ptr<Node> &a, const std::shared_ptr<Node> &b)
    {
        int rankA = nodeRank(a);
        int rankB = nodeRank(b);

        if (rankA != rankB)
            return rankA < rankB;

        const auto* varA = a->asSymbol();
        const auto* varB = b->asSymbol();

        if(!varA||!varB)
            return a->toString()<b->toString();

        if (varA->getName() != varB->getName())
            return varA->getName() < varB->getName();

        if (varA->getDegree() != varB->getDegree())
            return varA->getDegree() < varB->getDegree();

        if(varA->getCoeff()!=varB->getCoeff())
        {
            return varA->getCoeff() < varB->getCoeff();
        }
        
        return varA->rawToString()<varB->rawToString(); // string comparison as absolute fallback
    }

    
    Environment::Environment(const std::vector<std::string> &tokens)
    {
        for(size_t i=0;i<tokens.size();i++)
        {
            const std::string& arg = tokens[i];
            double val = 0.0;
            if(i+1 < tokens.size())
            {
                try
                {
                    val = std::stod(tokens[i+1]);
                }
                catch (const std::exception& e) {
                    map[arg] = 0.0;
                    continue;
                }
                map[arg] = val;
                i++;
            }
            else
                try
                {
                    val = std::stod(arg);
                    break;
                }
                catch (const std::exception& e) {
                    map[arg] = 0.0;
                    break;
                }

        }
    }

    std::shared_ptr<Node> Symbol::simplify()
    {
        if(coefficient == 0)
            return std::make_shared<NumberNode>(0);
        else if(degree == 0)
        {
            return std::make_shared<NumberNode>(coefficient);
        }

        return this->shared_from_this();
    }

    double VariableNode::evaluate(const Environment &env) const
    {
        double value = env[this->getName()];
        return this->getCoeff()*std::pow(value,this->getDegree());
    }
    

    bool OperatorNode::identical(const Node *other) const
    {
        if(!this->sameOperator(other))
            return false;

        const auto* otherOp = other->asSymbol()->asOperator();
        if(arguments.size()!=otherOp->arguments.size())
            return false;
        for(int i=0;i<arguments.size();i++)
        {
            if(!arguments[i]->identical(otherOp->arguments[i]))
                return false;
        }
        return true;
    }
    OperatorNode::OperatorNode(const std::vector<NodePtr>& args, std::shared_ptr<const OpDefinition>& op)
    : Symbol(op ? op->getName() : "OP", 1, 1), op(op), arguments(args)
    {}

    OperatorNode::OperatorNode(const std::vector<NodePtr>& args, std::shared_ptr<const OpDefinition>& op, double coeff, double degree)
        : Symbol(op ? op->getName() : "OP", coeff, degree), op(op), arguments(args)
    {}

    OperatorNode::OperatorNode(const OperatorNode& other) 
        : Symbol(other), op(other.op)
    {
        arguments.reserve(other.arguments.size());
        for(const auto& arg : other.arguments) {
            if(arg) {
                arguments.push_back(arg->clone());
            } else {
                arguments.push_back(nullptr);
            }
        }
    }
    // method that applies properties of the operator to streamline its arguments
    // for example, sorts the arguments of commutative operators
    void OperatorNode::unify()
    {

        std::cout << "Pre unification: " << postFixNotation() << std::endl;
        // unify children (during expansion/simplification, they may become disorderly again despite the bottom up approach)
        for(auto& arg : arguments)
            if(auto innerOp = toOperator(arg))
                innerOp->unify();


        bool associative = op->hasProperty(OpProperty::TRUE_ASSOCIATIVE);
        bool commutative = op->hasProperty(OpProperty::COMMUTATIVE);

        if(associative)
        {
            std::vector<NodePtr> flattenedArgs = {};
            flattenedArgs.reserve(arguments.size() * 2);
            for(const auto& arg : arguments)
            {
                auto op = toOperator(arg);
                if(op && op->sameOperator(this))
                {
                    OperatorNode* innerOp = arg->asSymbol()->asOperator();
                    for(const auto& innerArg : innerOp->arguments)
                    {
                        flattenedArgs.push_back(innerArg);
                    }
                }
                else
                    flattenedArgs.push_back(arg);
            }
            arguments = std::move(flattenedArgs);
        }

        if(commutative)
        {
            std::sort(arguments.begin(), arguments.end(), canonicalLess);
        }
        std::cout << "Post unification: " << postFixNotation() << std::endl;
    }

    std::shared_ptr<Node> OperatorNode::simplify()
    {
        std::cout << "Pre simplification: " << postFixNotation() << std::endl;
        unify();
        // simplify locally then try recursively
        auto cleaned = op->applySimplification(std::static_pointer_cast<OperatorNode>(this->shared_from_this()));
        auto cleanedOp = toOperator(cleaned);
        
        if(!cleanedOp)
            return cleaned;

        std::vector<NodePtr> simplifiedArgs;
        simplifiedArgs.reserve(cleanedOp->getArguments().size());

        for(auto& arg : cleanedOp->getArguments())
        {
            if(auto innerOperator = toOperator(arg)) {
                simplifiedArgs.push_back(innerOperator->simplify());
            } else {
                simplifiedArgs.push_back(arg);
            }
        }
        
        cleanedOp->acquireArguments(simplifiedArgs);
        cleanedOp->unify();

        auto final = cleanedOp->getOpDefinition()->applySimplification(cleanedOp);
        std::cout << "Post simplification: " << final->postFixNotation() << std::endl;
        return final;
    }



    NodePtr OperatorNode::expand()
    {
        std::cout << "Pre expansion: " << postFixNotation() << std::endl;
        unify();

        // perform shallow cleaning
        auto cleaned = op->applySimplification(std::static_pointer_cast<OperatorNode>(this->shared_from_this()));
        auto cleanedOp = toOperator(cleaned);
        if(!cleanedOp) // simplified into an atomic node
            return cleaned;

        std::vector<NodePtr> expandedArgs;
        expandedArgs.reserve(cleanedOp->arguments.size());

        for(auto& arg : cleanedOp->getArguments())
        {
            if(auto innerOperator = toOperator(arg)) {
                auto expanded = innerOperator->expand();
                expandedArgs.push_back(expanded);
            } else {
                expandedArgs.push_back(arg);
            }
        }
        cleanedOp->acquireArguments(expandedArgs);
        cleanedOp->unify();

        auto final = cleanedOp->getOpDefinition()->applyExpansion(std::static_pointer_cast<OperatorNode>(cleanedOp));
        std::cout << "Post expansion: " << final->postFixNotation() << std::endl;
        return final;
    }
    std::shared_ptr<Node> OperatorNode::canonicalize()
    {
        std::shared_ptr<Node> expanded = expand(); 
        
        auto opPtr = toOperator(expanded);
        if(!opPtr)
            return expanded;
            
        return opPtr->simplify(); 
    }

    const std::string OperatorNode::rawToString() const
    {
        return op->toString(arguments);
    }
    double OperatorNode::evaluate(const Environment &env) const
    {
        std::vector<double> values = {};
        values.reserve(arguments.size());

        for(const auto& arg:arguments)
        {
            values.push_back(arg->evaluate(env));
        }
        double baseResult = op->evaluate(values);
        return this->getCoeff() * std::pow(baseResult, this->getDegree());
    }

};