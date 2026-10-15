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
            else if(auto* symbol = arg->asSymbol())
            {
                double symCoeff = symbol->getCoeff();
                double symDegree = symbol->getDegree();
                std::string identifier;
                if(const auto* innerOp = symbol->asOperator())
                    identifier = innerOp->rawToString();
                else
                    identifier = symbol->getName();
                symbolMap.insertOrAccumulate(identifier, symbol, [symCoeff, symDegree](double& coeff, double& degree){

                    coeff *= symCoeff;
                    degree += symDegree;
                });
            }
        }
        
        const auto& map = symbolMap.get();
        std::vector<NodePtr> newArgs;

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
                    
                    if (auto* symbolInterface = termNode->asSymbol()) 
                    {
                        symbolInterface->setCoeff(coeff);
                        symbolInterface->setDegree(degree);
                    }
                    
                    newArgs.push_back(std::move(termNode));
                }
            }
        }
        args = std::move(newArgs);
        // apply constant
        opNode->setCoeff(opNode->getCoeff() * symbolMap.getConstant());

        // node reduction checks
        if(args.size() == 0)
            return NodeFactory::makeNumber(std::pow(opNode->getCoeff(), opNode->getDegree()));
        else if (args.size() == 1)
        {

            NodePtr singleChild = std::move(args.front());
            if (auto* childSymbol = singleChild->asSymbol())
            {   // apply the metadata of this operator
                childSymbol->setCoeff(childSymbol->getCoeff() * opNode->getCoeff());
                childSymbol->setDegree(childSymbol->getDegree() * opNode->getDegree());
                return singleChild;
            }
            else if (auto* childNum = singleChild->asNumber())
            {
                return NodeFactory::makeNumber(
                    opNode->getCoeff() * std::pow(childNum->getValue(), opNode->getDegree())
                );
            }
        }
        
        return opNode->shared_from_this(); 
    }

    NodePtr Multiply::applyExpansion(OpPtr opNode) const
    {
        std::vector<NodePtr> argsCopy = opNode->getArguments();
        NodePtr firstAdditionNode; 

        if (opNode->getCoeff() != 1.0)
        {
            argsCopy.push_back(NodeFactory::makeNumber(opNode->getCoeff()));
        }

        // find the first addition node
        for(size_t i = 0; i < argsCopy.size(); i++)
        {
            if(const auto innerOp = toOperator(argsCopy[i]))
            {
                if(innerOp->getName() == "+" && innerOp->getCoeff() == 1) 
                {
                    firstAdditionNode = std::move(argsCopy[i]);
                    argsCopy.erase(argsCopy.begin() + i);
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
        if (additionToDistribute->getCoeff() != 1.0)
        {
            argsCopy.push_back(NodeFactory::makeNumber(additionToDistribute->getCoeff()));
        }
        std::vector<NodePtr> newTermsToAdd;
        newTermsToAdd.reserve(additionToDistribute->getArguments().size());

        for(const auto& arg : additionToDistribute->getArguments())
        {
            std::vector<NodePtr> factorPool = argsCopy;
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