#include "../../include/NodeFactory.hpp"
#include "Cos.hpp"
#include "TrigHelper.hpp"


namespace AST
{
Cos::Cos():
    OpDefinition(std::vector<OpProperty>{OpProperty::RIGHT_ASSOCIATIVE}){}

NodePtr Cos::localCollapseStructure(OpPtr opNode) const
{
    return opNode;
}

InfFloat Cos::approximate(const std::vector<InfFloat>& operands) const
{
    if(operands.size()<1)
        return 0;
    return mp::cos(operands[0]);
}

std::string Cos::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

NodePtr Cos::simplify(OpPtr opNode) const
{
    auto child = opNode->getArgument(0);
    auto childOp = SoftCasting::toOperator(child);
    
    PiRational angle;
    extractPiAngle(child, angle);

    if(angle.isValid && angle.denominator != 0)
    { // try to use the unit circle to evaluate exactly
        InfInt N = angle.numerator;
        InfInt D = angle.denominator;

        int sign = 1;
        InfInt lookupN = N;

        if(N == 0 || N == 2 * D) // 0*pi, 2*pi
            return NodeFactory::makeNumber(1);
        if(N == D) // 1/1*pi
            return NodeFactory::makeNumber(-1);
        // if in quadrant 1 already, do nothing
        // [0,pi/2]

        // quadrant 2, (pi/2,pi]
        if (N * 2 > D && N < D) {
            // -cos(pi-theta)
            lookupN = D - N;
            sign = -1;
        }
        // quadrant 3 (pi,3pi/2]
        else if (N >= D && N * 2 <= 3 * D) {
            // -cos(theta - pi)
            lookupN = N - D;
            sign = -1;
        }
        // quadrant 4, (3pi/2,2pi)
        else if (N * 2 > 3 * D && N < 2 * D) {
            // cos(2pi - theta)
            lookupN = (2 * D) - N;
            sign = 1;
        }

        // reduce to bare minimal coeffecients
        InfInt finalG = gcd(lookupN, D);
        lookupN /= finalG;
        D /= finalG;

         // map to exact unit circle nodes based on the reduced denominator
        if (D == 2 && lookupN == 1) {       // cos(pi/2) = 0
            return NodeFactory::makeNumber(0);
        }
        if (D == 6 && lookupN == 1) {       // cos(pi/6) = sqrt(3)/2

            auto sqrt3 = NodeFactory::makeOperator("sqrt", {NodeFactory::makeNumber(3)});
            auto expr = NodeFactory::makeOperator("/", {sqrt3, NodeFactory::makeNumber(2)});
            if (sign == -1) {
                return NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), expr});
            }
            return expr;
        }
        if (D == 4 && lookupN == 1) {       // cos(pi/4) = sqrt(2)/2
            auto sqrt2 = NodeFactory::makeOperator("sqrt", {NodeFactory::makeNumber(2)});
            auto expr = NodeFactory::makeOperator("/", {sqrt2, NodeFactory::makeNumber(2)});
            if (sign == -1) {
                return NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), expr});
            }
            return expr;
        }
        if (D == 3 && lookupN == 1) {       // cos(pi/3) = 1/2
            return NodeFactory::makeNumber(0.5 * sign); 
        }

    }

    if (!childOp) return opNode;
   // cos(-x) = cos(x)
    if (childOp->getName() == "*")
    {
        auto coeff = childOp->getArgument(0);
        if(auto numeric = SoftCasting::toRawNumber(coeff))
            if(numeric->getValue() < 0)
            {
                auto positiveCoeff = NodeFactory::makeNumber(-numeric->getValue());
                
                std::vector<NodePtr> remainingArgs = childOp->getArguments();
                remainingArgs[0] = positiveCoeff;
                
                auto newInternalMul = OperatorNode::simplify(NodeFactory::makeOperator("*", remainingArgs));
                opNode = NodeFactory::getWithArguments(opNode,std::vector<NodePtr>{newInternalMul});
            }
    }
    return opNode;
}

NodePtr Cos::expand(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
