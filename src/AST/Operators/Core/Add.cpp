#include "../../include/NodeFactory.hpp"
#include "Add.hpp"

namespace AST
{
    Add::Add():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){}

    InfFloat Add::approximate(const std::vector<InfFloat>& values) const
    {
        
        InfFloat f(0);

        for (const auto& arg : values) {
            f+=arg;
        }
        return f;
    }

    NodePtr Add::simplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        constexpr int IDENTITY_ELEMENT = 0;

        std::map<std::pair<NodePtr,InfRational>, InfRational> map; 
        InfRational constant(IDENTITY_ELEMENT); // default value

        for(auto arg : oldArgs)
        {
            InfRational coeff(1); 
            InfRational degree(1);
            NodePtr nodeToInsert = arg;

            if(auto rational = SoftCasting::toRational(arg)) {
                constant += rational->getFraction();
                continue;
            }

            // extract coefficient
            if(auto op = SoftCasting::toOperator(nodeToInsert); op && op->getName() == "*") {
                if(auto coeffNode = SoftCasting::toRational(op->getArgument(0))) {
                    coeff = coeffNode->getFraction();
                    nodeToInsert = op->remakeWithoutArgument(0);
                    
                    // unwrap if it's one node left
                    if(auto checkOp = SoftCasting::toOperator(nodeToInsert); checkOp && checkOp->getArguments().size() == 1) {
                        nodeToInsert = checkOp->getArgument(0);
                    }
                }
            }

            // extract degree (after coeffecient)
            if(auto op = SoftCasting::toOperator(nodeToInsert); op && op->getName() == "^") {
                auto exponent = op->getArgument(1);
                if(auto expRat = SoftCasting::toRational(exponent)) {
                    degree = expRat->getFraction();
                    nodeToInsert = op->getArgument(0); // strip to core base
                }
            }

            // now the node is in polynomial form, store by degree -> coeff mapping
            std::pair nodeAndDegree = {nodeToInsert, degree};
            map[nodeAndDegree] += coeff; 
        }

        

        // simplify the map
        std::map<std::pair<NodePtr,InfRational>, InfRational> simplifiedMap; 
        for(auto& [nodeAndDegree,coeff]:map)
        {
            const auto& node = nodeAndDegree.first;
            const auto& degree = nodeAndDegree.second;

            if(coeff == 0) // 0x^n -> 0
                continue;
            else if(degree == 0) { //n*x^0 -> n
                constant+=coeff;
                continue;
            }
            else if(degree == 1)
                if(auto rational = SoftCasting::toRational(node)) {
                    constant += coeff * rational->getFraction();
                    continue;
                }
            simplifiedMap[nodeAndDegree] = coeff; // copy nontrivial information
        }

        map.clear(); // clear the outdated map

        // rebuild the simplified arguments using the cached information
        std::vector<NodePtr> reconstructed;
        reconstructed.reserve(oldArgs.size());

        // if the constant will actually matter, insert it first
        if(constant != IDENTITY_ELEMENT)
            reconstructed.push_back(NodeFactory::makeNumber(constant));
        
        for(auto& [nodeAndDegree,coeff]:simplifiedMap)
        {
            const auto& node = nodeAndDegree.first;
            const auto& degree = nodeAndDegree.second;

            if(coeff == 1 && degree == 1)
                reconstructed.push_back(node);
            else if(coeff == 1 && degree != 1)
                reconstructed.push_back(NodeFactory::makeOperator("^",{node, NodeFactory::makeNumber(degree)}));
            else if(coeff != 1 && degree == 1)
                reconstructed.push_back(NodeFactory::makeOperator("*",{NodeFactory::makeNumber(coeff), node}));
            else
                reconstructed.push_back(NodeFactory::makeOperator("*",{NodeFactory::makeNumber(coeff),NodeFactory::makeOperator("^",{node, NodeFactory::makeNumber(degree)})}));
        }        

        opNode = NodeFactory::getWithArguments(opNode,reconstructed); // wash away the old structure at this node

        return localCollapseStructure(opNode); // final collapse handling in case of a malformed operation
        // particularly, constants that only flattened out during reconstruction, such as in the degree = 1 or degree = 0 case
    }
    NodePtr Add::expand(OpPtr opNode) const
    {
        return opNode->shared_from_this();
    }

    NodePtr Add::localCollapseStructure(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());
        InfRational constant(0);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toRational(node))
            {
                if(number->getFraction() == 0)
                    continue;
                constant += number->getFraction();
                continue;
            }

            newArgs.push_back(node);
        }

        if (newArgs.empty())
            return NodeFactory::makeNumber(constant);
        if(constant != 0)
            newArgs.push_back(NodeFactory::makeNumber(constant));
        if(newArgs.size() == 1)
            return newArgs[0]; // collapse to child

        opNode = NodeFactory::getWithArguments(opNode,newArgs);
        return opNode;
    }
    std::string Add::toString(const std::vector<NodePtr> &args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            str+=" + " + args[i]->toString();
        }

        return str;
    }

    OpPtr Add::buildInverseOf(OpPtr opToInvert) const
    {
        if(opToInvert->getDefinition()->getName() != getName())
            return nullptr; // not the same operator

        auto& args = opToInvert->getArguments();
        std::vector<NodePtr> newArgs;
        newArgs.reserve(args.size());

        // The addition operator is invertible by negating all of its arguments, 
        // so we can build the inverse by negating all of the arguments of the inputted operator

        // Adding the new operator and this operator will always simplify to 0, the addative identiy.
        for(const auto& arg: args)
        {
            auto negated = NodeFactory::makeOperator("*",{NodeFactory::makeNumber(-1),arg});
            newArgs.push_back(negated);
        }

        return NodeFactory::makeOperator(getName(),newArgs);
    }
}
