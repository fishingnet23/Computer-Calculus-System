#include "../../include/NodeFactory.hpp"
#include "Sin.hpp"
#include "TrigHelper.hpp"
namespace AST
{

Sin::Sin():
    OpDefinition(std::vector<OpProperty>{OpProperty::RIGHT_ASSOCIATIVE}){}

NodePtr Sin::localCollapseStructure(OpPtr opNode) const
{
    auto& args = opNode->getArguments();
    return opNode;
}

InfFloat Sin::approximate(const std::vector<InfFloat>& operands) const
{
    if(operands.size()<1)
        return 0;
    return mp::sin(operands[0]);
}

std::string Sin::toString(const std::vector<NodePtr> &args) const
{
    return getName() + "(" + args[0]->toString() + ")";
}

NodePtr Sin::simplify(OpPtr opNode) const
{
    auto child = opNode->getArgument(0);
    auto childOp = SoftCasting::toOperator(child);
    
   

    PiRational angle;
    extractPiAngle(child, angle);

    if(angle.isValid && angle.denominator != 0)
    { // try to use the unit circle to evaluate exactly
        InfInt N = angle.numerator;
        InfInt D = angle.denominator;

        std::cout << "NUMERATOR " << N << std::endl;

        if (N == 0 || N == D || N == 2 * D) {
            return NodeFactory::makeNumber(0);
        }

        int sign = 1;
        InfInt lookupN = N;

        // if in quadrant 1 already, do nothing
        // [0,pi/2]

        // quadrant 2, (pi/2,pi]
        if (N * 2 > D && N < D) {
            // sin(pi - theta)
            lookupN = D - N;
        }
        // quadrant 3 (pi,3pi/2]
        else if (N >= D && N * 2 <= 3 * D) {
            // -sin(theta - pi)
            lookupN = N - D;
            sign = -1;
        }
        // quadrant 4, (3pi/2,2pi)
        else if (N * 2 > 3 * D && N < 2 * D) {
            // -sin(2pi - theta)
            lookupN = (2 * D) - N;
            sign = -1;
        }

        // reduce to bare minimal coeffecients
        InfInt finalG = gcd(lookupN, D);
        lookupN /= finalG;
        D /= finalG;

         // map to exact unit circle nodes based on the reduced denominator
        if (D == 2 && lookupN == 1) {       // sin(pi/2) = 1
            return NodeFactory::makeNumber(1 * sign);
        }
        if (D == 6 && lookupN == 1) {       // sin(pi/6) = 1/2
            return NodeFactory::makeNumber(0.5 * sign); 
        }
        if (D == 4 && lookupN == 1) {       // sin(pi/4) = sqrt(2)/2
            auto sqrt2 = NodeFactory::makeOperator("sqrt", {NodeFactory::makeNumber(2)});
            auto expr = NodeFactory::makeOperator("/", {sqrt2, NodeFactory::makeNumber(2)});
            if (sign == -1) {
                return NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), expr});
            }
            return expr;
        }
        if (D == 3 && lookupN == 1) {       // sin(pi/3) = sqrt(3)/2
            auto sqrt3 = NodeFactory::makeOperator("sqrt", {NodeFactory::makeNumber(3)});
            auto expr = NodeFactory::makeOperator("/", {sqrt3, NodeFactory::makeNumber(2)});
            if (sign == -1) {
                return NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), expr});
            }
            return expr;
        }

    }

    if (!childOp) return opNode;
   // sin(-x) = -sin(x)
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
                auto neg1 = NodeFactory::makeNumber(-1);
                return NodeFactory::makeOperator("*", {neg1, opNode});
            }
    }
    return opNode;
}

NodePtr Sin::expand(OpPtr opNode) const
{
    return opNode->shared_from_this();
}
}
