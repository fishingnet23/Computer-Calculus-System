#include "../include/NodeFactory.hpp"

namespace AST
{
    double Multiply::evaluate(const std::vector<double>& values) const
    {
        double res = 1;
        for(const auto& value:values)
        {
            res *= value;
        }
        return res;
    }
    NodePtr Multiply::applySymbolicIdentities(OpPtr opNode) const
    {

        return opNode->shared_from_this();
    }
    NodePtr Multiply::applySimplification(OpPtr opNode) const
    {
        auto identity = applySymbolicIdentities(opNode);
        if(!toOperator(identity))
            return identity;
        opNode = std::static_pointer_cast<OperatorNode>(identity);

        auto& args = opNode->getArguments();
        SymbolDataAccumulator symbolMap;
        symbolMap.setConstant(1); // need something to multiply with other constants, the default is zero

        for(const auto& arg : args)
        {
            if(const auto* number = arg->asNumber())
            {
                if(number->getValue() == 0)
                    return arg; // if there is a zero, the whole expression collapses

                symbolMap.setConstant(symbolMap.getConstant() * number->getValue());
            }
            else if(const auto& symbol = toSymbol(arg))
            {
                double symCoeff = 1;
                double symDegree = 1;
                std::string identifier;
                SymbolPtr finalNode = symbol;

                if(const auto& innerOp = toOperator(arg))
                {
                    if(innerOp->getName() == "^")
                    {
                        const auto& innerArgs = innerOp->getArguments();
                        if(innerArgs.size() != 2)
                            throw std::runtime_error("Malformed power node detected in multiplication!");
                        const auto& exponentNumber = toNumber(innerArgs[1]);
                        const auto& symbolBase = toSymbol(innerArgs[0]);
                        if(exponentNumber&&symbolBase)
                        {
                           symDegree = exponentNumber->getValue();
                           finalNode = symbolBase;
                        }
                    }

                    identifier = finalNode->toString();
                }
                else
                    identifier = finalNode->getName();
                symbolMap.insertOrAccumulate(identifier, finalNode, [symCoeff, symDegree](double& coeff, double& degree){

                    coeff *= symCoeff;
                    degree += symDegree;
                });
            }
        }
        
        const auto& map = symbolMap.get();
        std::vector<NodePtr> newArgs;
        if(symbolMap.getConstant() != 1.0)
            newArgs.push_back(NodeFactory::makeNumber(symbolMap.getConstant()));
        // rebuild args
        for(const auto& [name, OpData] : map)
        {
            {
                auto coeff = OpData.coeff;
                auto degree = OpData.degree;
                

                if(degree == 0)
                    symbolMap.setConstant(symbolMap.getConstant()*coeff);
                else
                {

                    NodePtr termNode = OpData.originalNode->clone(); 
                    NodePtr finalNode = termNode;
                    if(degree != 1)
                    {
                        finalNode = NodeFactory::makeOperator("^",{termNode, NodeFactory::makeNumber(degree)});
                    }
                    
                    newArgs.push_back(std::move(finalNode));
                }
            }
        }
        args = std::move(newArgs);
        // node reduction checks
        if(args.size() == 0)
            return NodeFactory::makeNumber(0);
        else if (args.size() == 1)
        {

            NodePtr singleChild = std::move(args.front());
            
            return singleChild;
        }
        
        return opNode->shared_from_this(); 
    }

    NodePtr Multiply::applyExpansion(OpPtr opNode) const
    {
        std::vector<NodePtr>& args = opNode->getArguments();
        NodePtr firstAdditionNode; 

        // find the first addition node
        for(size_t i = 0; i < args.size(); i++)
        {
            if(const auto innerOp = toOperator(args[i]))
            {
                if(innerOp->getName() == "+") 
                {
                    firstAdditionNode = args[i];
                    args.erase(args.begin() + i);
                    break;
                }
            }
        }

        if(!firstAdditionNode)
        {
            opNode->unify(); 
            return opNode->shared_from_this();
        }

        auto additionToDistribute = toOperator(firstAdditionNode);

        std::vector<NodePtr> newTermsToAdd;
        newTermsToAdd.reserve(additionToDistribute->getArguments().size());

        for(const auto& arg : additionToDistribute->getArguments())
        {
            // clone to recursively clone the remaining arguments
            auto cloned = opNode->clone();
            auto cloneOp = toOperator(cloned);
            std::vector<NodePtr>& factorPool = cloneOp->getArguments();
            factorPool.push_back(arg);
            
            auto internalMultiplication = NodeFactory::makeOperator("*", factorPool);
            std::cout << "INTERNAL MULTIPLICATION " << internalMultiplication->postFixNotation() << std::endl;
            newTermsToAdd.push_back(internalMultiplication);
        }

        auto newAddition = NodeFactory::makeOperator("+", newTermsToAdd);
        return newAddition; 
    }

    std::string Multiply::toString(const std::vector<NodePtr> & args) const
    {
        std::string str = "";
        if(args.size() == 0) // corrupted state
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            str+=" * " + args[i]->toString();
        }

        return str;
    }
}