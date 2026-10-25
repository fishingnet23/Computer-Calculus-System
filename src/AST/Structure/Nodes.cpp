#include "Nodes.hpp"
#include "OperatorDefinitions.hpp"
#include "../include/NodeFactory.hpp"

namespace AST
{
    SymbolPtr Casting::toSymbol(const NodePtr& node)
    {
        if (!node) return nullptr;
        return node->asSymbol()? std::static_pointer_cast<Symbol>(node):nullptr;
    }
    NumberPtr Casting::toNumber(const NodePtr& node)
    {
        if (!node) return nullptr;
        return node->asNumber()? std::static_pointer_cast<NumberNode>(node):nullptr;
    }

    VariablePtr Casting::toVariable(const SymbolPtr& node)
    {
        if (!node) return nullptr;
        return node->asVariable()? std::static_pointer_cast<VariableNode>(node):nullptr;
    }
    VariablePtr Casting::toVariable(const NodePtr& node)
    {
        if (!node) return nullptr;
        auto symbol = node->asSymbol();
        return (symbol && symbol->asVariable()) 
            ? std::static_pointer_cast<VariableNode>(node) 
            : nullptr;
    }
    OpPtr Casting::Casting::toOperator(const SymbolPtr& node)
    {
        if (!node) return nullptr;
        return node->asOperator()? std::static_pointer_cast<OperatorNode>(node):nullptr;
    }
    OpPtr Casting::Casting::toOperator(const NodePtr& node)
    {
        if (!node) return nullptr;
        return node->asSymbol() && node->asSymbol()->asOperator() 
            ? std::static_pointer_cast<OperatorNode>(node) 
            : nullptr;
    }
    int Node::nodeRank(const ConstNodePtr node)
    {
        if (node->type() == NodeType::Number)
            return 0;

        if (node->type() == NodeType::Variable)
            return 1;

        if (node->type() == NodeType::Operator)
            return 2;

        return 3;
    }
    bool Node::canonicalLess(const ConstNodePtr a, const ConstNodePtr b)
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

        return varA->toString()<varB->toString(); // string comparison as absolute fallback
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
        return this->shared_from_this();
    }

    double VariableNode::evaluate(const Environment &env) const
    {
        double value = env[this->getName()];
        return value;
    }
    

    bool OperatorNode::identical(const ConstNodePtr other) const
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

    OperatorNode::OperatorNode(const std::vector<NodePtr> &args, const std::shared_ptr<const OpDefinition> &op)
        : Symbol(op ? op->getName() : "OP"), op(op),arguments(args){}

    OperatorNode::OperatorNode(const std::vector<NodePtr>& args, const std::shared_ptr<const OpDefinition>& op, double coeff, double degree)
        : Symbol(op ? op->getName() : "OP"), op(op) ,arguments(args)
    {
        // to do: add clone to op because cant construct a pure virtual function
    }

    NodePtr OperatorNode::remakeWithArgsFromInterval(size_t start, size_t end) const
    {
        std::vector<NodePtr> args;
        args.reserve(end-start);

        for(size_t i = start;i<end;i++)
            args.push_back(arguments[i]);

        auto res = NodeFactory::makeOperatorPtr(name,args);
        return res->getDefinition()->localSimplification(res);
    }

    NodePtr OperatorNode::remakeWithoutArgument(size_t elementIndex) const
    {
        std::vector<NodePtr> args;
        args.reserve(arguments.size() - 1);

        for(int i=0;i<arguments.size();i++)
        {
            if(i==elementIndex)
                continue;
            args.push_back(arguments[i]);
        }
        auto res = NodeFactory::makeOperatorPtr(name,args);
        return res->getDefinition()->localSimplification(res);
    }

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

        if (Config::DEBUG_MODE) std::cout << "Pre unification: " << getPostFix() << std::endl;
        // unify children (during expansion/simplification, they may become disorderly again despite the bottom up approach)
        for(auto& arg : arguments)
            if(auto innerOp = Casting::toOperator(arg))
                innerOp->unify();


        bool associative = op->hasProperty(OpProperty::TRUE_ASSOCIATIVE);
        bool commutative = op->hasProperty(OpProperty::COMMUTATIVE);

        if(associative)
        {
            std::vector<NodePtr> flattenedArgs = {};
            flattenedArgs.reserve(arguments.size() * 2);
            for(const auto& arg : arguments)
            {
                auto op = Casting::toOperator(arg);
                if(op && op->sameOperator(this->shared_from_this()))
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
        if(Config::DEBUG_MODE) std::cout << "Post unification: " << getPostFix() << std::endl;
    }

    std::shared_ptr<Node> OperatorNode::simplify()
    {
        if (Config::DEBUG_MODE) std::cout << "Pre simplification: " << getPostFix() << std::endl;
        unify();
        // simplify locally then try recursively
        auto cleaned = op->localSimplification(std::static_pointer_cast<OperatorNode>(this->shared_from_this()));
        auto cleanedOp = Casting::toOperator(cleaned);
        
        if(!cleanedOp)
            return cleaned;

        std::vector<NodePtr> simplifiedArgs;
        simplifiedArgs.reserve(cleanedOp->getArguments().size());

        for(auto& arg : cleanedOp->getArguments())
        {
            if(auto innerOperator = Casting::toOperator(arg)) {
                simplifiedArgs.push_back(innerOperator->simplify());
            } else {
                simplifiedArgs.push_back(arg);
            }
        }
        
        cleanedOp->setArguments(simplifiedArgs);
        cleanedOp->unify();
    
        // second shallow cleaning, ensures compressions that can only be found after simplifying children are found
        auto postChildClean = cleanedOp->getDefinition()->localSimplification(cleanedOp);
        auto postChildCleanOp = Casting::toOperator(postChildClean);
        
        if(!postChildCleanOp)
            return postChildClean;
        
        auto final = postChildCleanOp->getDefinition()->applySimplification(postChildCleanOp);
        auto finalOp = Casting::toOperator(final);
        if(!finalOp)
            return final;

       finalOp->unify(); // final unification
        if (Config::DEBUG_MODE) std::cout << "Post simplification: " << final->getPostFix() << std::endl;
        return final;
    }



    NodePtr OperatorNode::expand()
    {
        if (Config::DEBUG_MODE) std::cout << "Pre expansion: " << getPostFix() << std::endl;
        unify();

        // perform shallow cleaning
        auto cleaned = op->localSimplification(std::static_pointer_cast<OperatorNode>(this->shared_from_this()));
        auto cleanedOp = Casting::toOperator(cleaned);
        if(!cleanedOp) // simplified into an atomic node
            return cleaned;

        std::vector<NodePtr> expandedArgs;
        expandedArgs.reserve(cleanedOp->arguments.size());

        for(auto& arg : cleanedOp->getArguments())
        {
            if(auto innerOperator = Casting::toOperator(arg)) {
                auto expanded = innerOperator->expand();
                expandedArgs.push_back(expanded);
            } else {
                expandedArgs.push_back(arg);
            }
        }
        cleanedOp->acquireArguments(expandedArgs);
        cleanedOp->unify();

        // second shallow cleaning, ensures compressions that can only be found after expanding children are found
        auto postChildClean = cleanedOp->getDefinition()->localSimplification(cleanedOp);
        auto postChildCleanOp = Casting::toOperator(postChildClean);
        
        if(!postChildCleanOp)
            return postChildClean;
        
        auto final = postChildCleanOp->getDefinition()->applyExpansion(postChildCleanOp);
        auto finalOp = Casting::toOperator(final);
        if(!finalOp)
            return final;

       finalOp->unify(); // final unification

        if (Config::DEBUG_MODE) std::cout << "Post expansion: " << final->getPostFix() << std::endl;
        return final;
    }
    std::shared_ptr<Node> OperatorNode::canonicalize()
    {
        std::shared_ptr<Node> expanded = expand(); 
        
        auto opPtr = Casting::toOperator(expanded);
        if(!opPtr)
            return expanded;
            
        return opPtr->simplify(); 
    }

    const std::string OperatorNode::toString() const
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
        return baseResult;
    }

};