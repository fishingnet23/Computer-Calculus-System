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

    Node::ConstPolynomial Node::extractMetadata(ConstNodePtr node)
    {   

        if(node->type() != NodeType::Operator)
            return {node,1.0,1.0}; // signature is {term, degree, coeff}
        
        const auto* op = node->asSymbol()->asOperator();

        if(op->getName() == "^")
        {   if(const auto* exp = op->getArgument(1)->asNumber())
                return {op->getArgument(0), exp->getValue(),1};
            else
                return {node,1,1}; // complex exponent, like 2^x or x^(x+1)
        }

        if(op->getName() == "*")
        {
            const auto* coeff = op->getArgument(0)->asNumber();
            if(!coeff) // case with no constant multiple
                return {node,1,1};
            if(op->getArguments().size() != 2) // if the multiplication is beyond binary, don't bother
                return {op->remakeWithoutArgument(0), 1.0, coeff->getValue()};

            auto& innerArg = op->getArgument(1);
            const auto* innerSym = innerArg->asSymbol(); 
            if(!innerSym) // malformed multiplication, prob two numbers
            {
                if(const auto* innerNum = innerArg->asNumber())
                    return {NodeFactory::makeNumber(innerNum->getValue() * coeff->getValue()),1,1};
                return {node,1,1};
            }
                
            const auto* innerOp = innerSym->asOperator();

            if(!innerOp) // handle the (coeff * x) case
                return {op->getArgument(1), 1.0, coeff->getValue()};

            if(innerOp->getName() == "^") // assume ^ is not malformed and is exactly binary
            {
                auto meta = extractMetadata(innerArg);
                return {innerOp->getArgument(0), meta.degree, coeff->getValue()}; // returns <base, degree, coeff> from a node of the form coeff*(base)^degree
            }

            return {op->remakeWithoutArgument(0), 1.0, coeff->getValue()};
            
        }
        return {node,1.0,1.0};
    }

    int Node::nodeRank(ConstNodePtr node)
    {
        if (node->type() == NodeType::Number)
            return 0;
        else if (node->type() == NodeType::Variable)
            return 1;
        else if(node->type() == NodeType::Operator)
            return 2;
        else
            return 9999; //fallback
    }

    // returns a negative number if a<b, 0 if a=b, positive if a>b (In the structural sense, not nesecarily numerically)
    int Node::compare(ConstNodePtr a, ConstNodePtr b)
    {
        // flatten coeff*(base)^degree for easy comparison by base then degree then coeff
        int rankA = nodeRank(a);
        int rankB = nodeRank(b);

        if (rankA != rankB)
            return rankA - rankB;
        
        // flatten coeff*(base)^degree for easy comparison by base then degree then coeff
        auto polyA = extractMetadata(a); 
        auto polyB = extractMetadata(b);

        // recalculate rank
        rankA = nodeRank(polyA.node);
        rankB = nodeRank(polyB.node);

        if (rankA != rankB)
            return rankA - rankB;
        
        
        if (polyA.node->type() == NodeType::Number) {
            // for this case, ignore coeffecient / degree since they are irrelevant
            return polyA.node->asNumber()->getValue() - polyB.node->asNumber()->getValue();
        }

        if (polyA.node->type() == NodeType::Variable) 
        {
            int comp = polyA.node->asSymbol()->getName().compare(polyB.node->asSymbol()->getName());
            if(comp != 0)
                return comp;
            comp = polyA.degree - polyB.degree;
            if(comp != 0)
                return comp;
            
            return polyA.coeff - polyB.coeff; // ultimate fallback, will return 0 if identical
        }

        if(polyA.node->type() == NodeType::Operator)
        {
            int comp = polyA.node->asSymbol()->getName().compare(polyB.node->asSymbol()->getName());
            if(comp != 0)
                return comp;
            comp = polyA.degree - polyB.degree;
            if(comp != 0)
                return comp;
            comp =  polyA.coeff - polyB.coeff;
            if(comp != 0)
                return comp;
            
            const auto* opA = polyA.node->asSymbol()->asOperator();
            const auto* opB = polyB.node->asSymbol()->asOperator();

            size_t minSize = std::min(opA->getArguments().size(),opB->getArguments().size());
            for(size_t i=0;i<minSize;i++)
            {
                comp = compare(opA->getArgument(i),opB->getArgument(i));
                if(comp != 0) // if they aren't equal, short circuit, otherwise keep looking
                    return comp;
            }
            
            if(opA->getArguments().size() != opB->getArguments().size())
                return opA->getArguments().size() - opB->getArguments().size(); // less arguments < more arguments

            return 0; // operators are structurally identical
            
        }
        return 0; // truly equal
    }

    bool Node::canonicalLess(ConstNodePtr a, ConstNodePtr b)
    {
        return compare(a,b) < 0; // will return false in the case of equal, to maintain strict ordering
    }
    bool Node::canonicalMore(ConstNodePtr a, ConstNodePtr b)
    {
        return compare(a,b) > 0; // will return false in the case of equal, to maintain strict ordering
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

    NodePtr Symbol::simplify()
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
            auto format = op->getSortingFormat();

            if(format == SyntaxRule::SortingFormat::ASCEND)
                std::sort(arguments.begin(), arguments.end(), canonicalLess);
            else
                std::sort(arguments.begin(), arguments.end(), canonicalMore);

        }
        if(Config::DEBUG_MODE) std::cout << "Post unification: " << getPostFix() << std::endl;
    }

    NodePtr OperatorNode::localSimplify(){return op->localSimplification(std::static_pointer_cast<OperatorNode>(this->shared_from_this()));}

    NodePtr OperatorNode::simplify()
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
    NodePtr OperatorNode::canonicalize()
    {
        NodePtr expanded = expand(); 
        
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
        std::string str = toString();
        
        if (const double* val = env.lookup(str))
            return *val;

        std::vector<double> values = {};
        values.reserve(arguments.size());

        for(const auto& arg:arguments)
        {
            values.push_back(arg->evaluate(env));
        }
        double baseResult = op->evaluate(values,env);
        return baseResult;
    }

};