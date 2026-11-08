#include "Multiply.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
    Multiply::Multiply():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){}

    InfFloat Multiply::approximate(const std::vector<InfFloat>& values) const
    {
        
        InfFloat f(1);

        for (const auto& arg : values) {
            f*=arg;
        }
        return f;
    }
    NodePtr Multiply::simplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        constexpr int IDENTITY_ELEMENT = 1;
        constexpr int ABSORBING_ELEMENT = 0;

        std::map<NodePtr, NodePtr> degreeMap; 

        for(auto arg : oldArgs)
        {
            if(auto rational = SoftCasting::toRational(arg)) {
                if(rational->getFraction() == ABSORBING_ELEMENT)
                    return NodeFactory::makeNumber(ABSORBING_ELEMENT);
            }

            NodePtr degree = NodeFactory::makeNumber(1);
            NodePtr nodeToInsert = arg;

            // look for exponents
            if(auto op = SoftCasting::toOperator(arg))
            {
                if(op->getName() == "^")
                {
                    auto base = op->getArgument(0);
                    auto exponent = op->getArgument(1);
                    
                        degree = exponent;
                        nodeToInsert = base;            
                }
            }

            // check if the base is a number
            if(auto rational = SoftCasting::toRational(nodeToInsert))
            {
                auto frac = rational->getFraction();
                auto absNumerator = mp::abs(numerator(frac));
                auto absDenominator = mp::abs(denominator(frac));


                // normalize all rationals into a standard form so symbolic comparrison can account for reciprocals
                // 1/2 -> 2^-1, 3/5 -> (5/3)^-1, 5/3 -> 5/3 (since 5>3)
                if(absDenominator > absNumerator)
                {
                    frac = InfRational(denominator(frac),numerator(frac));
                    degree = NodeFactory::makeOperator("unary-",{degree});
                    nodeToInsert = NodeFactory::makeNumber(frac);
                }

                // if(degree == 1 && map.find(nodeToInsert) == map.end()) 
                // {
                //     constant *= rational->getFraction();
                //     continue; // Safe to consume globally
                // }
            }

            // update the map
            if(degreeMap.find(nodeToInsert) == degreeMap.end())
                degreeMap[nodeToInsert] = degree;
            else
                degreeMap[nodeToInsert] = NodeFactory::makeOperator("+",{degreeMap[nodeToInsert], degree}); // delegate to addition 
        }

        // simplify the map
        InfRational constant(1); // create constant accumulator
        std::map<NodePtr, NodePtr> simplifiedMap;
        for(auto& [node, degree] : degreeMap)
        {
            if(OpPtr degreeAddition = SoftCasting::toOperator(degree))
            {
                degree = OperatorNode::simplify(degreeAddition); // combine like exponents using addition directly
            }
            RationalPtr degreeRat = SoftCasting::toRational(degree);
            if(degreeRat && degreeRat->getFraction() == 0) { 
                continue;
            }
            // consume non symbolic constants
            else if(degreeRat && degreeRat->getFraction() == 1) {
                if(auto rational = SoftCasting::toRational(node)) {
                    constant *= rational->getFraction();
                    continue;
                }
            }
            else if(degreeRat && degreeRat->getFraction() == -1)
            {
                if(auto rational = SoftCasting::toRational(node)) {
                    constant *= InfRational(1,rational->getFraction());
                    continue;
                }
            }
            simplifiedMap[node] = degree; 
        }

        degreeMap.clear(); 

        // rebuild the simplified arguments
        std::vector<NodePtr> reconstructed;
        reconstructed.reserve(oldArgs.size());

        if(constant != IDENTITY_ELEMENT)
            reconstructed.push_back(NodeFactory::makeNumber(constant));
        
        for(auto& [node, degree] : simplifiedMap)
        {
            if(RationalPtr degreeRat = SoftCasting::toRational(degree); degreeRat && degreeRat->getFraction() == 1)
                reconstructed.push_back(node);
            else
                reconstructed.push_back(NodeFactory::makeOperator("^", {node, degree}));
        }        

        opNode = NodeFactory::getWithArguments(opNode, reconstructed); 
        return localCollapseStructure(opNode); 
    }
    NodePtr Multiply::expand(OpPtr opNode) const
    {

        // a(b+c) = a*b+a*c
        std::vector<std::vector<NodePtr>> currentCombinations = {{}};

        for(const auto& arg : opNode->getArguments())
        {
            std::vector<std::vector<NodePtr>> nextCombinations;
            
            if(const auto& addOp = SoftCasting::toOperator(arg); addOp && addOp->getName() == "+") // distribute over addition
            {
                for(const auto& existingTerms : currentCombinations)
                {
                    for(const auto& addArg : addOp->getArguments())
                    {
                        auto newCombination = existingTerms;
                        newCombination.push_back(addArg);
                        nextCombinations.push_back(newCombination);
                    }
                }
            }
            else
            {
                for(auto existingTerms : currentCombinations)
                {
                    existingTerms.push_back(arg);
                    nextCombinations.push_back(existingTerms);
                }
            }
            currentCombinations = std::move(nextCombinations);
        }

        std::vector<NodePtr> sumArguments;
        for(const auto& termGroup : currentCombinations)
        {
            if(termGroup.size() == 1)
            {
                sumArguments.push_back(termGroup[0]);
            }
            else
            {
                auto rawGroupNode = NodeFactory::makeOperator("*", termGroup);
                sumArguments.push_back(rawGroupNode);
            }
        }

        if(sumArguments.size() == 1)
        {
            return sumArguments[0];
        }

        return NodeFactory::makeOperator("+", sumArguments);
    }


    NodePtr Multiply::localCollapseStructure(OpPtr opNode) const
    {

        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());
        InfRational constantProduct = 1.0;

        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toRational(node))
            {
                if(number->getFraction() == 0)
                    return NodeFactory::makeNumber(0);
                if(number->getFraction() == 1)
                    continue;
                constantProduct *= number->getFraction();
                continue;
            }

            newArgs.push_back(node);
        }

        if (newArgs.empty())
            return NodeFactory::makeNumber(constantProduct);
        if(constantProduct != 1)
            newArgs.insert(newArgs.begin(), NodeFactory::makeNumber(constantProduct));
        if(newArgs.size() == 1)
            return newArgs[0]; // collapse to child

        opNode = NodeFactory::getWithArguments(opNode,newArgs);
        return opNode;
    }
std::string Multiply::toString(const std::vector<NodePtr> &args) const
{
    if (args.empty()) 
        return "";

    std::string str = "";

    auto precedence = getPrecedence();
    auto getWrappedString = [precedence](const NodePtr& node) {
        auto op = SoftCasting::toOperator(node);
        auto rat = SoftCasting::toRational(node);
        if ((rat && denominator(rat->getFraction())!=1 )|| (op && op->getDefinition()->getPrecedence() < precedence)) {
            return "(" + node->toString() + ")";
        }
        return node->toString();
    };

    str += getWrappedString(args[0]);

    for (size_t i = 1; i < args.size(); i++)
    {
        auto prevNumber = SoftCasting::toRawNumber(args[i-1]);
        auto currNumber = SoftCasting::toRawNumber(args[i]);
        auto currVar    = SoftCasting::toVariable(args[i]);
        auto currOp     = SoftCasting::toOperator(args[i]);

        std::string currStr = getWrappedString(args[i]);

        if (prevNumber && currVar) {
            str += currStr; 
        }
        else if (prevNumber && currOp && (currStr.front() == '(' || (currOp->getName() == "^" && SoftCasting::toSymbol(currOp->getArgument(0))))) {
            str += currStr;
        }
        else {
            str += "*" + currStr;
        }
    }

    return str;
}

OpPtr Multiply::buildInverseOf(OpPtr opToInvert) const
{
    if(opToInvert->getDefinition()->getName() != getName())
        return nullptr; // not the same operator

    auto& args = opToInvert->getArguments();
    std::vector<NodePtr> newArgs;
    newArgs.reserve(args.size());

    for(const auto& arg: args)
    {
        newArgs.push_back(NodeFactory::makeOperator("^",{arg,NodeFactory::makeNumber(-1)})); // a^(-1) = 1/a
    }

    return NodeFactory::makeOperator("*",newArgs);

}
}