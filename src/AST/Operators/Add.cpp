#include "../include/NodeFactory.hpp"

namespace AST
{
    double Add::evaluate(const std::vector<double>& values) const
    {
        double res = 0;
        for(const auto& value:values)
        {
            res += value;
        }
        return res;
    }
    NodePtr Add::applySymbolicIdentities(OpPtr opNode) const
    {
        // DISTRIBUTIVE PROPERTY
        if(opNode->getCoeff()!= 1.0 && opNode->getDegree() == 1.0)
        {
            for(auto& arg:opNode->getArguments())
            {
                if(auto* symbol = arg->asSymbol())
                {
                    symbol->setCoeff(symbol->getCoeff()*opNode->getCoeff());
                }
                else if(auto* number = arg->asNumber())
                {
                    number->setValue(number->getValue()*opNode->getCoeff());
                }
            }
        opNode->setCoeff(1);
        }

        

       simplifyArgsOf(opNode);
        return opNode->shared_from_this();
    }
    NodePtr Add::applySimplification(OpPtr opNode) const
    {
        auto identity = applySymbolicIdentities(opNode);
        if(!toOperator(identity))
            return identity;
        opNode = std::static_pointer_cast<OperatorNode>(identity);

        auto& args = opNode->getArguments();
        IndexedSymbolDataAccumulator coeffMap(IndexedSymbolDataAccumulator::IndexMode::ByDegree);
        
        for(const auto& arg : args)
        {
            if(const auto* number = arg->asNumber())
            {
                coeffMap.setConstant(coeffMap.getConstant() + number->getValue());
            }
            else if(const auto* symbol = arg->asSymbol())
            {
                double coeff = symbol->getCoeff();
                double degree = symbol->getDegree();
                std::string identifier;
                if(const auto* innerOp = symbol->asOperator())
                    identifier = innerOp->rawToString();
                else
                    identifier = symbol->getName();

                coeffMap.insertOrAccumulate(identifier, arg,coeff, degree);
            }
        }
        
        const auto& map = coeffMap.get();
        args.clear();
        // insert constant term
        if (coeffMap.getConstant() != 0.0)
        {
            args.push_back(NodeFactory::makeNumber(coeffMap.getConstant()));
        }

        // rebuild nonzero args
        for(const auto& [name, innerMap] : map)
        {
            for(const auto& [degree, OpData] : innerMap)
            {
                auto coeff = OpData.uniqueValue;
                if(coeff != 0.0)
                {

                    NodePtr termNode = OpData.originalNode->clone(); 
                    
                    if (auto* symbolInterface = termNode->asSymbol()) 
                    {
                        symbolInterface->setCoeff(coeff);
                        symbolInterface->setDegree(degree);
                    }
                    
                    args.push_back(std::move(termNode));
                }
            }
        }

        // node reduction checks
        if(args.size() == 0)
        {
            return NodeFactory::makeNumber(0);
        }
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

    NodePtr Add::applyExpansion(OpPtr opNode) const
    {
        return opNode->shared_from_this();
    }

    std::string Add::toString(const std::vector<NodePtr> & args) const
    {
        std::string str = "";
        if(args.size() == 0)
            return str;
        str += args[0]->toString();

        for(size_t i = 1;i<args.size();i++)
        {
            if(!(args[i]->asSymbol() && args[i]->asSymbol()->getCoeff()<0))
                str+=" + ";
            str+= args[i]->toString();
        }

        return str;
    }
}