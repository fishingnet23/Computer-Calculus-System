#include "../include/NodeFactory.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace AST
{
    namespace
    {
        constexpr double METADATA_IDENTITY = 1.0;

        bool hasDefaultMetadata(const OperatorNode& node)
        {
            return node.getCoeff() == METADATA_IDENTITY
                && node.getDegree() == METADATA_IDENTITY;
        }

        // Returns exponent * degree without modifying either input node.
        NodePtr scaleExponent(const NodePtr& exponent, double degree)
        {
            if (degree == METADATA_IDENTITY)
                return exponent->clone();

            NodePtr scaled = NodeFactory::makeOperator(
                "*",
                {
                    exponent->clone(),
                    NodeFactory::makeNumber(degree)
                }
            );

            return toOperator(scaled)->simplify();
        }
    }

    double Exponentiate::evaluate(const std::vector<double>& values) const
    {
        if (values.size() != 2)
            throw std::runtime_error(
                "Binary operator ^ doesn't have exactly 2 arguments!"
            );

        return std::pow(values[0], values[1]);
    }

    NodePtr Exponentiate::applySymbolicIdentities(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();

        if (args.size() != 2)
            throw std::runtime_error(
                "Binary operator ^ doesn't have exactly 2 arguments!"
            );

        auto baseOperator = toOperator(args[0]);

        if (!baseOperator || baseOperator->getName() != "^")
            return opNode->shared_from_this();

        if (!hasDefaultMetadata(*baseOperator))
            return opNode->shared_from_this();


        auto clonedBase = toOperator(args[0]);
        auto& innerArgs = clonedBase->getArguments();

        if (innerArgs.size() != 2)
            throw std::runtime_error(
                "Binary operator ^ doesn't have exactly 2 arguments!"
            );

        NodePtr newExponent = NodeFactory::makeOperator(
            "*",
            {
                innerArgs[1],
                args[1]
            }
        );

        innerArgs[1] = toOperator(newExponent)->simplify();
        args[0] = clonedBase;

        return opNode->shared_from_this();
    }

    NodePtr Exponentiate::applySimplification(OpPtr opNode) const
    {
        auto& values = opNode->getArguments();

        if (values.size() != 2)
            throw std::runtime_error(
                "Binary operator ^ doesn't have exactly 2 arguments!"
            );

        auto identity = applySymbolicIdentities(opNode);

        if (!toOperator(identity))
            return identity;

        opNode = std::static_pointer_cast<OperatorNode>(identity);

        auto base = values[0];
        auto exponent = values[1];


        auto* exponentNumber = exponent->asNumber();

        if (!exponentNumber)
            return opNode->shared_from_this();

        const double exponentValue = exponentNumber->getValue();

       
        const double effectiveExponent =
            exponentValue * opNode->getDegree();

        const double outerCoeff = opNode->getCoeff();

        if (auto* baseNumber = base->asNumber())
        {
            return NodeFactory::makeNumber(
                outerCoeff
                * std::pow(baseNumber->getValue(), effectiveExponent)
            );
        }


        if (auto originalSymbol = toSymbol(base))
        {


            const double baseCoeff = originalSymbol->getCoeff();
            const double baseDegree = originalSymbol->getDegree();

            originalSymbol->setCoeff(
                outerCoeff * std::pow(baseCoeff, effectiveExponent)
            );

            originalSymbol->setDegree(
                baseDegree * effectiveExponent
            );


            if (auto op = toOperator(originalSymbol))
                return op->simplify();

            return base;
        }

        return opNode->shared_from_this();
    }

    NodePtr Exponentiate::applyExpansion(OpPtr opNode) const
    {
        auto& args = opNode->getArguments();

        if (args.size() != 2)
            throw std::runtime_error(
                "Binary operator ^ doesn't have exactly 2 arguments!"
            );

        auto base = args[0];
        auto exponent = args[1];

        // (a*b)^x = a^x * b^x
        if (auto baseOperator = toOperator(base))
        {
            if (baseOperator->getName() == "*"
                && hasDefaultMetadata(*baseOperator) == true
                && !baseOperator->getArguments().empty())
            {
                const auto& innerArgs = baseOperator->getArguments();

                NodePtr distributedExponent =
                    scaleExponent(exponent, opNode->getDegree());

                std::vector<NodePtr> newArgs;
                newArgs.reserve(innerArgs.size());

                for (const auto& arg : innerArgs)
                {
                    newArgs.push_back(
                        NodeFactory::makeOperator(
                            "^",
                            {
                                arg,
                                distributedExponent->clone()
                            }
                        )
                    );
                }

                NodePtr multiplication =
                    NodeFactory::makeOperator("*", newArgs);

                auto multiplicationOperator = toOperator(multiplication);

                // Multiplication's coefficient is the final multiplier
                // of the entire distributed expression.
                multiplicationOperator->setCoeff(
                    multiplicationOperator->getCoeff()
                    * opNode->getCoeff()
                );

                return multiplicationOperator
                    ->getOpDefinition()
                    ->applyExpansion(multiplicationOperator);
            }
        }


        if (auto* expNumber = exponent->asNumber())
        {
            const double effectiveExponent =
                expNumber->getValue() * opNode->getDegree();

            if (effectiveExponent >= 2.0
                && std::isfinite(effectiveExponent)
                && effectiveExponent
                    < static_cast<double>(
                        std::numeric_limits<std::size_t>::max()
                    ))
            {
                const double integerPart = std::floor(effectiveExponent);
                const double remainderPart =
                    effectiveExponent - integerPart;

                const auto power =
                    static_cast<std::size_t>(integerPart);

                std::vector<NodePtr> mulArgs;
                mulArgs.reserve(
                    power + (remainderPart > 0.0 ? 1 : 0)
                );

                for (std::size_t i = 0; i < power; ++i)
                    mulArgs.push_back(base->clone());


                if (remainderPart > 0.0)
                {
                    mulArgs.push_back(
                        NodeFactory::makeOperator(
                            "^",
                            {
                                base->clone(),
                                NodeFactory::makeNumber(remainderPart)
                            }
                        )
                    );
                }

                NodePtr multiplication =
                    NodeFactory::makeOperator("*", mulArgs);

                auto multiplicationOperator = toOperator(multiplication);

                multiplicationOperator->setCoeff(
                    multiplicationOperator->getCoeff()
                    * opNode->getCoeff()
                );

                return multiplicationOperator
                    ->getOpDefinition()
                    ->applyExpansion(multiplicationOperator);
            }
        }

        return opNode->shared_from_this();
    }

    std::string Exponentiate::toString(
        const std::vector<NodePtr>& args
    ) const
    {
        std::string str;

        if (args.empty())
            return str;

        str += args[0]->toString();

        for (std::size_t i = 1; i < args.size(); ++i)
            str += " ^ " + args[i]->toString();

        return str;
    }
}
