#include "Nodes.hpp"
#include "OperatorDefinitions.hpp"
#include "../include/NodeFactory.hpp"

namespace AST
{

    SymbolPtr SoftCasting::toSymbol(const NodePtr node)
    {
        if (!node) return nullptr;
        return node->asSymbol()? std::static_pointer_cast<const Symbol>(node):nullptr;
    }
    NumberPtr SoftCasting::toNumber(const NodePtr node)
    {
        if (!node) return nullptr;
        return node->asNumber()? std::static_pointer_cast<const NumberNode>(node):nullptr;
    }

    RationalPtr SoftCasting::toRational(const NodePtr node)
    {
        if (!node) return nullptr;
        return node->asNumber()? node->asNumber()->asRational()? std::static_pointer_cast<const RationalNode>(node):nullptr:nullptr;
    }

    RationalPtr SoftCasting::toRational(const NumberPtr node)
    {
        if (!node) return nullptr;
        return node->asRational()? std::static_pointer_cast<const RationalNode>(node):nullptr;
    }

    FloatPtr SoftCasting::toFloat(const NodePtr node)
    {
        if (!node) return nullptr;
        return node->asNumber()? node->asNumber()->asFloat()? std::static_pointer_cast<const FloatNode>(node):nullptr:nullptr;
    }

    FloatPtr SoftCasting::toFloat(const NumberPtr node)
    {
        if (!node) return nullptr;
        return node->asFloat()? std::static_pointer_cast<const FloatNode>(node):nullptr;
    }

    VariablePtr SoftCasting::toVariable(const SymbolPtr node)
    {
        if (!node) return nullptr;
        return node->asVariable()? std::static_pointer_cast<const VariableNode>(node):nullptr;
    }
    VariablePtr SoftCasting::toVariable(const NodePtr node)
    {
        if (!node) return nullptr;
        auto symbol = node->asSymbol();
        return (symbol && symbol->asVariable()) 
            ? std::static_pointer_cast<const VariableNode>(node) 
            : nullptr;
    }
    OpPtr SoftCasting::SoftCasting::toOperator(const SymbolPtr node)
    {
        if (!node) return nullptr;
        return node->asOperator()? std::static_pointer_cast<const OperatorNode>(node):nullptr;
    }
    OpPtr SoftCasting::SoftCasting::toOperator(const NodePtr node)
    {
        if (!node) return nullptr;
        return node->asSymbol() && node->asSymbol()->asOperator() 
            ? std::static_pointer_cast<const OperatorNode>(node) 
            : nullptr;
    }

    Node::ConstPolynomial Node::extractMetadata(NodePtr node)
    {   
        auto one = NodeFactory::makeNumber(1);
        if(node->type() != NodeType::Operator)
            return {node,one,one}; // signature is {term, degree, coeff}
        
        const auto* op = node->asSymbol()->asOperator();

        if(op->getName() == "^")
        {   if(auto exp = SoftCasting::toNumber(op->getArgument(1)))
                return {op->getArgument(0), exp,one};
            else
                return {node,one,one}; // complex exponent, like 2^x or x^(x+1)
        }

        if(op->getName() == "*")
        {
            auto coeff = SoftCasting::toNumber(op->getArgument(0));
            if(!coeff) // case with no constant multiple
                return {node,one,one};
            if(op->getArguments().size() != 2) // if the multiplication is beyond binary, don't bother
                return {op->remakeWithoutArgument(0), one, coeff};

            auto& innerArg = op->getArgument(1);
            const auto* innerSym = innerArg->asSymbol(); 
            if(!innerSym) // malformed multiplication, prob two numbers
            {
                if(auto innerNum = SoftCasting::toNumber(innerArg))
                    return {NodeFactory::makeNumber(innerNum->getValue() * coeff->getValue()),one,one};
                return {node,one,one};
            }
                
            const auto* innerOp = innerSym->asOperator();

            if(!innerOp) // handle the (coeff * x) case
                return {op->getArgument(1), one, coeff};

            if(innerOp->getName() == "^") // assume ^ is not malformed and is exactly binary
            {
                auto meta = extractMetadata(innerArg);
                return {innerOp->getArgument(0), meta.degree, coeff}; // returns <base, degree, coeff> from a node of the form coeff*(base)^degree
            }

            return {op->remakeWithoutArgument(0), one, coeff};
            
        }
        return {node,one,one};
    }

    int Node::nodeRank(NodePtr node)
    {
        if (node->type() == NodeType::Scalar)
            return 0;
        else if (node->type() == NodeType::Variable)
            return 1;
        else if(node->type() == NodeType::Operator)
            return 2;
        else
            return 9999; //fallback
    }

    // returns a negative number if a<b, 0 if a=b, positive if a>b (In the structural sense, not nesecarily numerically)
    int Node::compare(NodePtr a, NodePtr b)
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
        
        
        if (polyA.node->type() == NodeType::Scalar) {
            // for this case, ignore coeffecient / degree since they are irrelevant
            return SoftCasting::toNumber(polyA.node)->compare(SoftCasting::toNumber(polyB.node));
        }

        if (polyA.node->type() == NodeType::Variable) 
        {
            int comp = polyA.node->asSymbol()->getName().compare(polyB.node->asSymbol()->getName());
            if(comp != 0)
                return comp;
            comp = polyA.degree->compare(polyB.degree);
            if(comp != 0)
                return comp;
            
            return polyA.coeff->compare(polyB.coeff); // ultimate fallback, will return 0 if identical
        }

        if(polyA.node->type() == NodeType::Operator)
        {
            int comp = polyA.node->asSymbol()->getName().compare(polyB.node->asSymbol()->getName());
            if(comp != 0)
                return comp;
            comp = polyA.degree->compare(polyB.degree);
            if(comp != 0)
                return comp;
            comp =   polyA.coeff->compare(polyB.coeff);
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

    bool Node::canonicalLess(NodePtr a, NodePtr b)
    {
        return compare(a,b) < 0; // will return false in the case of equal, to maintain strict ordering
    }
    bool Node::canonicalMore(NodePtr a, NodePtr b)
    {
        return compare(a,b) > 0; // will return false in the case of equal, to maintain strict ordering
    }

    
    Environment::Environment(const std::vector<std::string> &tokens)
    {
        for(size_t i=0;i<tokens.size();i++)
        {
            const std::string& arg = tokens[i];
            InfFloat val = 0.0;
            if(i+1 < tokens.size())
            {
                try
                {
                    val = InfFloat(tokens[i+1]);
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
                    val = InfFloat(arg);
                    break;
                }
                catch (const std::exception& e) {
                    map[arg] = 0.0;
                    break;
                }

        }
    }

    bool RationalNode::identical(const NodePtr other) const
    {
        auto rat = SoftCasting::toRational(other);
        if(!rat)
            return false; // for now

        return mp::abs(fraction - rat->fraction) < Config::EPSILON;
    }

    NumberPtr VariableNode::evaluate(const Environment &env) const
    {
        InfFloat value = env[this->getName()];
        return NodeFactory::makeNumber(value);
    }
    

    bool OperatorNode::identical(const NodePtr other) const
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


    NodePtr OperatorNode::remakeWithArgsFromInterval(size_t start, size_t end) const
    {
        std::vector<NodePtr> args;
        args.reserve(end-start);

        for(size_t i = start;i<end;i++)
            args.push_back(arguments[i]);

        auto res = NodeFactory::makeOperator(name,args);
        return res->getDefinition()->complexLocalSimplify(res);
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
        auto res = NodeFactory::makeOperator(name,args);
        return res->getDefinition()->complexLocalSimplify(res);
    }

    OperatorNode::OperatorNode(const OperatorNode& other) 
        : Symbol(other), op(other.op) ,arguments(other.arguments){}
    // method that applies properties of the operator to streamline its arguments
    // for example, sorts the arguments of commutative operators
    OpPtr OperatorNode::unify(OpPtr opToSimplify)
    {

        if (Config::DEBUG_MODE) std::cout << "Pre unification: " << opToSimplify->getPostFix() << std::endl;
        // unify children (during expansion/simplification, they may become disorderly again despite the bottom up approach)

        std::vector<NodePtr> unifiedArgs;
        unifiedArgs.reserve(opToSimplify->getArguments().size());
        for(auto& arg : opToSimplify->getArguments())
            if(auto innerOp = SoftCasting::toOperator(arg))
                unifiedArgs.push_back(unify(innerOp));
            else
                unifiedArgs.push_back(arg);

        auto unified = NodeFactory::getWithArguments(opToSimplify,unifiedArgs);

        bool associative = opToSimplify->getDefinition()->hasProperty(OpProperty::TRUE_ASSOCIATIVE);
        bool commutative = opToSimplify->getDefinition()->hasProperty(OpProperty::COMMUTATIVE);


        if(associative)
        {
            std::vector<NodePtr> flattenedArgs = {};
            flattenedArgs.reserve(unifiedArgs.size() * 2);
            for(const auto& arg : unifiedArgs)
            {
                auto op = SoftCasting::toOperator(arg);
                if(op && op->sameOperator(opToSimplify))
                {
                    const OperatorNode* innerOp = arg->asSymbol()->asOperator();
                    for(const auto& innerArg : innerOp->arguments)
                    {
                        flattenedArgs.push_back(innerArg);
                    }
                }
                else
                    flattenedArgs.push_back(arg);
            }
            opToSimplify = NodeFactory::getWithArguments(unified,flattenedArgs);
        }

        if(commutative)
        {
            auto format = opToSimplify->getDefinition()->getSortingFormat();
            std::vector<NodePtr> sortedArgs = opToSimplify->getArguments(); // non reference
            if(format == SyntaxRule::SortingFormat::ASCEND)
                std::sort(sortedArgs.begin(), sortedArgs.end(), canonicalLess);
            else
                std::sort(sortedArgs.begin(), sortedArgs.end(), canonicalMore);
            opToSimplify = NodeFactory::getWithArguments(opToSimplify,sortedArgs);
        }
        if(Config::DEBUG_MODE) std::cout << "Post unification: " << opToSimplify->getPostFix() << std::endl;

        return opToSimplify;
    }

    NodePtr OperatorNode::localSimplify(OpPtr opToSimplify)
    {
        return opToSimplify->getDefinition()->complexLocalSimplify(opToSimplify);
    }

    NodePtr OperatorNode::simplify(OpPtr opToSimplify)
    {
        if (Config::DEBUG_MODE) std::cout << "Pre simplification: " << opToSimplify->getPostFix() << std::endl;
        opToSimplify = unify(opToSimplify);
        // simplify locally then try recursively
        auto cleaned = localSimplify(opToSimplify);
        opToSimplify = SoftCasting::toOperator(cleaned);
        
        if(!opToSimplify)
            return cleaned;

        std::vector<NodePtr> simplifiedArgs;
        simplifiedArgs.reserve(opToSimplify->getArguments().size());

        for(auto& arg : opToSimplify->getArguments())
        {
            if(auto innerOperator = SoftCasting::toOperator(arg)) {
                simplifiedArgs.push_back(simplify(innerOperator));
            } else {
                simplifiedArgs.push_back(arg);
            }
        }
        
        opToSimplify = NodeFactory::getWithArguments(opToSimplify,simplifiedArgs);
        opToSimplify = unify(opToSimplify);
    
        // second shallow cleaning, ensures compressions that can only be found after simplifying children are found
        auto postChildClean = opToSimplify->localSimplify(opToSimplify);
        opToSimplify = SoftCasting::toOperator(postChildClean);
        
        if(!opToSimplify)
            return postChildClean;

        auto final = opToSimplify->getDefinition()->complexSimplify(opToSimplify);
        opToSimplify = SoftCasting::toOperator(final);
        if(!opToSimplify)
            return final;
    
       opToSimplify = unify(opToSimplify); // final unification

        if (Config::DEBUG_MODE) std::cout << "Post simplification: " << final->getPostFix() << std::endl;
        return opToSimplify;
    }



    NodePtr OperatorNode::expand(OpPtr opToExpand)
    {
        if (Config::DEBUG_MODE) std::cout << "Pre expansion: " << opToExpand->getPostFix() << std::endl;
        opToExpand = unify(opToExpand);
        // simplify locally then try recursively
        auto cleaned = localSimplify(opToExpand);
        opToExpand = SoftCasting::toOperator(cleaned);
        
        if(!opToExpand)
            return cleaned;

        std::vector<NodePtr> simplifiedArgs;
        simplifiedArgs.reserve(opToExpand->getArguments().size());

        for(auto& arg : opToExpand->getArguments())
        {
            if(auto innerOperator = SoftCasting::toOperator(arg)) {
                simplifiedArgs.push_back(expand(innerOperator));
            } else {
                simplifiedArgs.push_back(arg);
            }
        }
        
        opToExpand = NodeFactory::getWithArguments(opToExpand,simplifiedArgs);
        opToExpand = unify(opToExpand);
    
        // second shallow cleaning, ensures compressions that can only be found after expanding children are found
        auto postChildClean = opToExpand->getDefinition()->complexLocalSimplify(opToExpand);
        opToExpand = SoftCasting::toOperator(postChildClean);
        
        if(!opToExpand)
            return postChildClean;

        auto final = opToExpand->getDefinition()->complexExpansion(opToExpand);
        opToExpand = SoftCasting::toOperator(final);
        if(!opToExpand)
            return final;

       opToExpand = unify(opToExpand); // final unification

        if (Config::DEBUG_MODE) std::cout << "Post expansion: " << final->getPostFix() << std::endl;
        return opToExpand;
    }
    NodePtr OperatorNode::canonicalize(OpPtr opToSimplify)
    {
        NodePtr expanded = expand(opToSimplify); 
        
        opToSimplify = SoftCasting::toOperator(expanded);
        if(!opToSimplify)
            return expanded;
            
        return simplify(opToSimplify); 
    }

    const std::string OperatorNode::toString() const
    {
        return op->toString(arguments);
    }
    NumberPtr OperatorNode::evaluate(const Environment &env) const
    {
        std::string str = toString();
        
        if (const InfFloat* val = env.lookup(str))
            return NodeFactory::makeNumber(*val);

        std::vector<NumberPtr> values = {};
        values.reserve(arguments.size());

        for(const auto& arg:arguments)
        {
            if(auto symb = SoftCasting::toSymbol(arg))
                values.push_back(symb->evaluate(env));
            else if(auto num = SoftCasting::toNumber(arg))
                values.push_back(num);
        }

        return op->evaluate(values,env);
    }

    const std::string FloatNode::toString() const  
    {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(0) << val; // dump it all
        std::string raw_str = ss.str();

        if (raw_str.length() > 12) {
            std::stringstream compressed;
            
            compressed << raw_str[0] << ".";
            
            compressed << raw_str.substr(1, 5);
            
            std::string leading = compressed.str();
            leading.erase(leading.find_last_not_of('0') + 1, std::string::npos);
            if (leading.back() == '.') {
                leading.pop_back();
            }

            size_t exponent = raw_str.length() - 1;
            
            return leading + "...e+" + std::to_string(exponent);
        }

        return raw_str;
    }

    bool FloatNode::identical(const NodePtr other) const
    {
        if(auto o = SoftCasting::toFloat(other))
            return this->val == o->val;
        return false;
    }

};