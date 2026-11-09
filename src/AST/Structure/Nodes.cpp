#include "Nodes.hpp"
#include "OperatorDefinitions.hpp"
#include "../include/NodeFactory.hpp"
#include "../Parsing/Parser.hpp"
#include <set>

namespace AST
{

    SymbolPtr SoftCasting::toSymbol(const NodePtr node)
    {
        if (!node) return nullptr;
        const Symbol* rawSymbol = node->asSymbol();
        if (!rawSymbol) return nullptr;
        return std::shared_ptr<const Symbol>(node, rawSymbol);
    }

    NumberPtr SoftCasting::toRawNumber(const NodePtr node)
    {
        if (!node) return nullptr;
        if (node->asSymbol()) return nullptr; // No SymbolicConstant nodes!
        return SoftCasting::toNumber(node);
    }

    NumberPtr SoftCasting::toNumber(const NodePtr node)
    {
        if (!node) return nullptr;
        const NumberNode* rawNumber = node->asNumber();
        if (!rawNumber) return nullptr;
        return std::shared_ptr<const NumberNode>(node, rawNumber);
    }

    RationalPtr SoftCasting::toRational(const NodePtr node)
    {
        if (!node) return nullptr;
        const NumberNode* rawNumber = node->asNumber();
        if (!rawNumber) return nullptr;
        const RationalNode* rational = rawNumber->asRational();
        if (!rational) return nullptr;
        return std::shared_ptr<const RationalNode>(node, rational);
    }

    RationalPtr SoftCasting::toRational(const NumberPtr node)
    {
        if (!node) return nullptr;
        const RationalNode* rational = node->asRational();
        if (!rational) return nullptr;
        return std::shared_ptr<const RationalNode>(node, rational);
    }

    VariablePtr SoftCasting::toVariable(const SymbolPtr node)
    {
        if (!node) return nullptr;
        const VariableNode* var = node->asVariable();
        if (!var) return nullptr;
        return std::shared_ptr<const VariableNode>(node, var);
    }

    VariablePtr SoftCasting::toVariable(const NodePtr node)
    {
        if (!node) return nullptr;
        const Symbol* symbol = node->asSymbol();
        if (!symbol) return nullptr;
        const VariableNode* var = symbol->asVariable();
        if (!var) return nullptr;
        return std::shared_ptr<const VariableNode>(node, var);
    }

    OpPtr SoftCasting::toOperator(const SymbolPtr node)
    {
        if (!node) return nullptr;
        const OperatorNode* op = node->asOperator();
        if (!op) return nullptr;
        return std::shared_ptr<const OperatorNode>(node, op);
    }

    OpPtr SoftCasting::toOperator(const NodePtr node)
    {
        if (!node) return nullptr;
        const Symbol* symbol = node->asSymbol();
        if (!symbol) return nullptr;
        const OperatorNode* op = symbol->asOperator();
        if (!op) return nullptr;
        return std::shared_ptr<const OperatorNode>(node, op);
    }


    Node::ConstPolynomial Node::extractMetadata(NodePtr node)
    {   
        auto one = NodeFactory::makeNumber(1);
        if(node->type() != NodeType::Operator)
            return {node,one,one}; // signature is {term, degree, coeff}
        
        const auto* op = node->asSymbol()->asOperator();

        if(op->getName() == "^")
        {   if(auto exp = SoftCasting::toRawNumber(op->getArgument(1)))
                return {op->getArgument(0), exp,one};
            else
                return {node,one,one}; // complex exponent, like 2^x or x^(x+1)
        }

        if(op->getName() == "*")
        {
            auto coeff = SoftCasting::toRawNumber(op->getArgument(0));
            if(!coeff) // case with no constant multiple
                return {node,one,one};
            if(op->getArguments().size() != 2) // if the multiplication is beyond binary, don't bother
                return {op->remakeWithoutArgument(0), one, coeff};

            auto& innerArg = op->getArgument(1);
            const auto* innerSym = innerArg->asSymbol(); 
            if(!innerSym) // malformed multiplication, prob two numbers
            {
                if(auto innerNum = SoftCasting::toRawNumber(innerArg))
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
    std::map<NodePtr, NodePtr> mapping;

    for (size_t i = 0; i < tokens.size() - 1; i += 2)
    {
        NodePtr lhs = Parser(tokens[i]).buildAST();
        NodePtr rhs = Parser(tokens[i + 1]).buildAST();

        if (!lhs || !lhs->isSubstitutable()) {
            throw std::runtime_error("Invalid assignment target! Left-hand side must be substitutable.");
        }

        auto [iterator, success] = mapping.insert({lhs, rhs});
        if (!success) {
            throw std::runtime_error("Duplicate mapping detected! Target already assigned.");
        }
    }

    if (tokens.size() > 1 && tokens.size() % 2 == 1)
    {
        NodePtr lhs = Parser(tokens[tokens.size() - 1]).buildAST();
        
        if (!lhs || !lhs->isSubstitutable()) {
            throw std::runtime_error("Invalid assignment target! Left-hand side must be substitutable.");
        }

        auto [iterator, success] = mapping.insert({lhs, NodeFactory::makeNumber(0)});
        if (!success) {
            throw std::runtime_error("Duplicate mapping detected! Target already assigned.");
        }
    }

    setMap(mapping);
}


    Environment::Environment(const std::map<NodePtr, NodePtr> &mapping){setMap(mapping);}

    NodePtr Environment::lookup(const std::string &str) const
    {
        auto node = Parser(str).buildAST();
        return lookup(node);
    }

    void Environment::setMap(const std::map<NodePtr, NodePtr> &inputs) {

        std::map<NodePtr, NodePtr> mapping;
        
        for (const auto& [lhs, rhs] : inputs) {
            if (!lhs || !lhs->isSubstitutable()) {
                throw std::runtime_error("Invalid assignment target! Left-hand side must be substitutable.");
            }

            auto [iterator, success] = mapping.insert({lhs, rhs});
            if (!success) {
                throw std::runtime_error("Duplicate mapping detected! Variable or operator already assigned.");
            }
        }

        std::set<NodePtr> visited;
        std::set<NodePtr> inProgress;

        auto hasInfiniteLoop = [&](auto& self, const NodePtr& currentVar) -> bool {
            if (!currentVar) return false;
            
            if (inProgress.count(currentVar)) return true;
            if (visited.count(currentVar)) return false;

            inProgress.insert(currentVar);

            auto it = mapping.find(currentVar);
            if (it != mapping.end()) {
                const NodePtr& expression = it->second;

                for (const auto& [otherVar, _] : mapping) {
                    // dont check against yourself
                    if (currentVar == otherVar) continue;

                    if (expression && expression->owns(otherVar)) {
                        if (self(self, otherVar)) return true;
                    }
                }
            }

            inProgress.erase(currentVar);
            visited.insert(currentVar);
            return false;
        };

        // check against cycles that can't be resolved in a finite amount of steps
        for (const auto& [var, _] : mapping) {
            if (hasInfiniteLoop(hasInfiniteLoop, var)) {
                throw std::runtime_error("Infinite evaluation loop detected! Do better.");
            }
        }

        this->map = mapping;
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
        return res->getDefinition()->localCollapseStructure(res);
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
        return res->getDefinition()->localCollapseStructure(res);
    }

    OperatorNode::OperatorNode(const OperatorNode& other) 
        : Symbol(other), op(other.op) ,arguments(other.arguments){}
    bool OperatorNode::owns(const NodePtr other) const
    {
        if(this == other.get())
            return true;
        for(const auto& arg:arguments)
        {
            if(arg->owns(other))
                return true;
        }
        return false;
    }
    bool OperatorNode::isSubstitutable() const
    {
        return op->hasProperty(OpProperty::FUNCTIONAL_MAPPING);
    }
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
        return opToSimplify->getDefinition()->localCollapseStructure(opToSimplify);
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

        auto final = opToSimplify->getDefinition()->simplify(opToSimplify);
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
        auto postChildClean = opToExpand->getDefinition()->localCollapseStructure(opToExpand);
        opToExpand = SoftCasting::toOperator(postChildClean);
        
        if(!opToExpand)
            return postChildClean;

        auto final = opToExpand->getDefinition()->expand(opToExpand);
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

    NodePtr OperatorNode::factorize(OpPtr opToFactorize)
    {
        opToFactorize = unify(opToFactorize);

        std::vector<NodePtr> factorizedArgs;
        factorizedArgs.reserve(opToFactorize->getArguments().size());

        for(auto& arg : opToFactorize->getArguments())
        {
            if(auto innerOperator = SoftCasting::toOperator(arg)) {
                factorizedArgs.push_back(factorize(innerOperator));
            } else {
                factorizedArgs.push_back(arg);
            }
        }

        opToFactorize = NodeFactory::getWithArguments(opToFactorize,factorizedArgs);
        opToFactorize = unify(opToFactorize);
        auto res = opToFactorize->getDefinition()->factorize(opToFactorize);
        opToFactorize = SoftCasting::toOperator(res);
        if(!opToFactorize)
            return res;
        opToFactorize = unify(opToFactorize);

        return opToFactorize;
    }

    InfFloat OperatorNode::approximate(OpPtr opToApproximate)
    {
        std::vector<InfFloat> approximateArgs; // they will be NumberPtrs
        approximateArgs.reserve(opToApproximate->getArguments().size());
        for(auto& arg : opToApproximate->getArguments())
        {
            if(auto innerOperator = SoftCasting::toOperator(arg)) {
                approximateArgs.push_back(approximate(innerOperator));
            } else if(auto innerNum = SoftCasting::toNumber(arg)){
                approximateArgs.push_back(innerNum->getValue());
            }
            else
                throw std::runtime_error("Cannot approximate: '" + arg->toString() + "' as a floating point number.");
        }

        auto finalApprox = opToApproximate->getDefinition()->approximate(approximateArgs);

        return finalApprox;
    }

    const std::string OperatorNode::toString() const
    {
        return op->toString(arguments);
    }

    bool OperatorNode::sameOperator(const NodePtr other) const
    {
        auto sym = SoftCasting::toSymbol(other);
        return sym && getName() == sym->getName();
    }

};