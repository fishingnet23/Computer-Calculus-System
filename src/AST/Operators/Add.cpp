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
            NodePtr strippedArgument;
            if(const auto* number = arg->asNumber())
            {
                coeffMap.setConstant(coeffMap.getConstant() + number->getValue());
                continue; // not a symbol so shouldnt be accumulated
            }
            else if(const auto& symbol = toSymbol(arg))
            { // needs reworking to ensure correct degree respect, theory is that it confuses x*x with degree 1
                double coeff = 1;
                double degree = 1;
                std::string identifier;
                
                if(const auto& op = toOperator(arg))
                {
                    if(op->getName() == "*")
                    {
                        if(op->getArguments().size() < 2)
                            throw std::runtime_error("Corrupted multiplication detected!");
                        NodePtr constant = op->getArguments()[0];
                        const NumberNode* number = constant->asNumber();
                        if(number)
                        {
                            coeff = number->getValue();
                            std::vector<NodePtr> reducedArgs = reduceToSymbols(op);
                            if(reducedArgs.size() == 1)
                                strippedArgument = reducedArgs[0]; // if just one operand was symbolic
                            else if(reducedArgs.empty()) // the operator was just hiding constants
                            {
                                double value = op->evaluate(Environment()); // safe to just evaluate with no variables
                                coeffMap.setConstant(coeffMap.getConstant() + value);
                                continue; // skip this as it's not a symbol, it's a hidden constant
                            }
                            else
                                strippedArgument = NodeFactory::makeOperator("*", reducedArgs);
                        }
                        else
                        {
                            identifier = op->toString();
                            strippedArgument = arg;
                        }
                    }
                    else
                    {
                        identifier = op->toString();
                        strippedArgument = arg;
                    }
                }
                else
                {
                    identifier = symbol->getName();
                    strippedArgument = arg;
                }

                coeffMap.insertOrAccumulate(identifier, strippedArgument, coeff, degree);
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
                    
                    NodePtr multiplied;
                    if(coeff!=1)
                        multiplied = NodeFactory::makeOperator("*",{NodeFactory::makeNumber(coeff),termNode});
                    else
                        multiplied = termNode;
                    args.push_back(std::move(multiplied));
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
            return singleChild;
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
            str+=" + " + args[i]->toString();
        }

        return str;
    }
}