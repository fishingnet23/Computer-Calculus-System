#include "Multiply.hpp"
#include "../../include/NodeFactory.hpp"

namespace AST
{
    Multiply::Multiply():OpDefinition({OpProperty::COMMUTATIVE,OpProperty::TRUE_ASSOCIATIVE}){}

    NumberPtr Multiply::evaluate(const std::vector<NumberPtr>& values, const Environment& env) const
    {
        bool allRational = true;
        for (const auto& arg : values) {
            if (!SoftCasting::toRational(arg)) {
                allRational = false;
                break;
            }
        }
        if (allRational) {
            InfRational r(1, 1);
            for (const auto& arg : values) {
                r *= SoftCasting::toRational(arg)->getFraction();
            }
            return NodeFactory::makeNumber(r);
        }
        
        InfFloat f(1);

        for (const auto& arg : values) {
            if (auto rat = SoftCasting::toRational(arg)) {
                f *= rat->getFraction().convert_to<InfFloat>();
            } 
            else if (auto f2 = SoftCasting::toFloat(arg)) {
                f *= f2->getFloat();
            } 
            else {
                f *= InfFloat(arg->getValue());
            }
        }

        return NodeFactory::makeNumber(f);
    }
    NodePtr Multiply::complexSimplify(OpPtr opNode) const
    {
        auto& oldArgs = opNode->getArguments();

        SymbolDataAccumulator coeffMap;
        coeffMap.setConstant(1);
        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toNumber(node))
            {
                if(number->getValue() == 0)
                    return NodeFactory::makeNumber(0);
                coeffMap.setConstant(coeffMap.getConstant()*number->getValue());
                continue;
            }
            InfFloat degree(1);
            if(const auto& symbol = SoftCasting::toSymbol(node))
            {
                SymbolPtr final = symbol;
                if(const auto& innerOp = SoftCasting::toOperator(node))
                {
                    auto& innerArgs = innerOp->getArguments();
                    if(innerOp->getName() == "^")
                    {    
                        // assume exponentiation is not malformed and that it is already canonicalized
                        if(const auto& expNode = SoftCasting::toNumber(innerArgs[1])) // check if the exponent is numeric 
                        {
                            // strip it
                            degree = expNode->getValue();
                            final = SoftCasting::toSymbol(innerArgs[0]);
                        }
                    }
                    
                }
                coeffMap.insertOrAccumulate(final->getPostFix(),final,[degree](InfFloat& accumCoeff,InfFloat& accumDeg)
                {
                    accumDeg += degree; // (a^b)*(a^c) = a^(b+c) 
                },1,degree);
            }
            
            // unknowns are ignored, though they shouldn't exist
        }

        std::vector<NodePtr> reconstructed;
        reconstructed.reserve(coeffMap.get().size() + 1);
        
        if(coeffMap.getConstant()!= 1)
            reconstructed.push_back(NodeFactory::makeNumber(coeffMap.getConstant()));
        
        for(const auto& [name,data] :coeffMap.get())
        {
            NodePtr nodeToMultiply;
            if(data.degree == 0)
                nodeToMultiply = NodeFactory::makeNumber(1); // placeholder, removed later
            else if(data.degree == 1)
                nodeToMultiply = data.originalNode;
            else
            {
                nodeToMultiply = NodeFactory::makeOperator("^",{data.originalNode,NodeFactory::makeNumber(data.degree)});
            }

            reconstructed.push_back(nodeToMultiply);
        }

        opNode = NodeFactory::getWithArguments(opNode,reconstructed); // wash away the old structure at this node

        return complexLocalSimplify(opNode); // final collapse handling in case of a malformed multiplication / final simplification
    }

    NodePtr Multiply::complexExpansion(OpPtr opNode) const
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


    NodePtr Multiply::complexLocalSimplify(OpPtr opNode) const
    {

        auto& oldArgs = opNode->getArguments();

        std::vector<NodePtr> newArgs;
        newArgs.reserve(oldArgs.size());
        InfFloat constantProduct = 1.0;

        for(const auto& node: oldArgs)
        {
            if(const auto& number = SoftCasting::toNumber(node))
            {
                if(number->getValue() == 0)
                    return NodeFactory::makeNumber(0);
                if(number->getValue() == 1)
                    continue;
                constantProduct *= number->getValue();
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
        if (op && op->getDefinition()->getPrecedence() < precedence) {
            return "(" + node->toString() + ")";
        }
        return node->toString();
    };

    str += getWrappedString(args[0]);

    for (size_t i = 1; i < args.size(); i++)
    {
        auto prevNumber = SoftCasting::toNumber(args[i-1]);
        auto currNumber = SoftCasting::toNumber(args[i]);
        auto currVar    = SoftCasting::toVariable(args[i]);
        auto currOp     = SoftCasting::toOperator(args[i]);

        std::string currStr = getWrappedString(args[i]);

        if (prevNumber && currVar) {
            str += currStr; 
        }
        else if (prevNumber && currOp && (currStr.front() == '(' || currOp->getName() == "^")) {
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