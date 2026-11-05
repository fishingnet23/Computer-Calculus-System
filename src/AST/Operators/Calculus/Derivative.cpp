#include "../../include/NodeFactory.hpp"
#include "Derivative.hpp"


namespace AST
{


Derivative::Derivative():OpDefinition({OpProperty::RIGHT_ASSOCIATIVE}){}

NumberPtr AST::Derivative::evaluate(const std::vector<NumberPtr> &values, const Environment& env) const
{
    throw std::runtime_error("Higher order operations cannot be evaluated directly, without substitution!");
}




NodePtr AST::Derivative::complexSimplify(OpPtr opNode) const
{
    auto& arguments = opNode->getArguments();

    if(arguments.size() != 2)
        throw std::runtime_error("Malformed differentiation! Cannot differentiate without both an operand and a variable to differentiate with respect to!");

    auto function = arguments[0];
    auto withRespectTo = Casting::toVariable(arguments[1]);

    if(!withRespectTo) // the operator is malformed, the engine will not deal with derivatives with respect to functions
        return opNode;


    return opNode;
}

NodePtr AST::Derivative::complexExpansion(OpPtr opNode) const
{
    auto& arguments = opNode->getArguments();

    if(arguments.size() != 2)
        throw std::runtime_error("Malformed differentiation! Cannot differentiate without both an operand and a variable to differentiate with respect to!");

    auto function = Casting::toOperator(arguments[0]);
    auto withRespectTo = Casting::toVariable(arguments[1]);

    if(!function || !withRespectTo) // there is nothing to expand, or the operator is just malformed
        return opNode;

    if(function->getName() == "+") // first property of linearity, d/dx(u+w) = d/dx(x) + d/dx(y)
    {
        auto& innerArgs = function->getArguments();
        std::vector<NodePtr> newArgs;
        newArgs.reserve(innerArgs.size());
        for(const auto& arg:innerArgs)
        {
            newArgs.push_back(NodeFactory::makeOperator("derivative",std::vector<NodePtr>{arg,withRespectTo})); 
        }
        function = NodeFactory::getWithArguments(function,newArgs);
        return OperatorNode::expand(function); // keep expanding
    }

    if(function->getName() == "^")
    // general exponent rule, I didn't learn this one in school
    // I wondered if there was a rule for a general exponentiation u^v, where u and v are both functions of x,
    // and it is: d/dx(u^v) = v * u^(v-1) * d/dx(u) + u^v * ln(u) * d/dx(v)
    {
        auto base = function->getArgument(0);
        auto exp = function->getArgument(1);

        auto exponentMinusOne = NodeFactory::makeOperator("+",{NodeFactory::makeNumber(-1),exp}); // exp-1
        
        auto powerRule = NodeFactory::makeOperator("*", // d/dx(u^n) = n * x^(n-1) * d/dx(u), the power rule where n is a constant and u is a function of x
            {
            exp, 
            NodeFactory::makeOperator("^",{base,exponentMinusOne}),
            NodeFactory::makeOperator("derivative",{base,withRespectTo})
            });
        
        auto exponentiationRule = NodeFactory::makeOperator("*", { // d/dx(n^v) = n^v * ln(n) * d/dx(v), the exponentiation rule where n is a constant and v is a function of x
            function,
            NodeFactory::makeOperator("ln", {base}),
            NodeFactory::makeOperator("derivative",{exp,withRespectTo}) 
        });

        auto combined = NodeFactory::makeOperator("+",std::vector<NodePtr>{powerRule,exponentiationRule});
        return OperatorNode::expand(combined); // keep expanding
    }
    else if(function->getName() == "*") 
    // good old product rule, unlike what I learned in school, it can be applied even on something like 5x
    // but since multiplication is flattened, I use the n-ary product rule
    // d/dx(u*v*w...) = d/dx(u)*v*w + u*d/dx(v)*w + u*v*d/dx(u) ...
    {
        auto& innerArgs = function->getArguments();
        std::vector<NodePtr> termsToAdd;
        termsToAdd.reserve(innerArgs.size());


        for(size_t i=0;i<innerArgs.size();i++)
        {
            auto newArgs = function->getArguments(); // not the original function's args, a temp copy of the vector itself

            auto i_thTerm = function->getArgument(i);
            newArgs[i] = NodeFactory::makeOperator("derivative", {i_thTerm,withRespectTo});
            auto newMulOp = NodeFactory::getWithArguments(function,newArgs); // don't overrite the old function reference
            termsToAdd.push_back(newMulOp);
        }

        auto addition = NodeFactory::makeOperator("+",termsToAdd);
        return OperatorNode::expand(addition);
    }



    // general chain rule for when applicable
    NodePtr outerDerivative = nullptr;
    NodePtr innerArgument = nullptr;

    if(function->getArguments().size() == 1)
    {
        innerArgument = function->getArgument(0);
        const std::string& opName = function->getName();

        if(opName == "sin")
        {
            // d/du(sin(u)) = cos(u)
            outerDerivative = NodeFactory::makeOperator("cos", {innerArgument});
        }
        else if(opName == "cos")
        {
            // d/du(cos(u)) = -1 * sin(u)
            auto sinNode = NodeFactory::makeOperator("sin", {innerArgument});
            outerDerivative = NodeFactory::makeOperator("*", {NodeFactory::makeNumber(-1), sinNode});
        }
    }

    if(function->getName() == "log") // d/dx(log(x,b)) = 1/(xlnx)
    {
        innerArgument = function->getArgument(0);
        auto baseNode = function->getArgument(1); 
        
        auto lnBase = NodeFactory::makeOperator("ln", {baseNode}); 
        auto oneOverLnBase = NodeFactory::makeOperator("^", {lnBase, NodeFactory::makeNumber(-1)});
        auto innerInverse = NodeFactory::makeOperator("^", {innerArgument, NodeFactory::makeNumber(-1)});
        
        outerDerivative = NodeFactory::makeOperator("*", {oneOverLnBase, innerInverse});
    }

    if(outerDerivative && innerArgument) // automatic chain rule when applicable
    {
        auto innerDerivative = NodeFactory::makeOperator("derivative", {innerArgument, withRespectTo});

        // f'(u) * d/dx(u)
        auto fullChainRuleTree = NodeFactory::makeOperator("*", {outerDerivative, innerDerivative});

        // keep expanding
        return OperatorNode::expand(fullChainRuleTree);
    }


    return opNode; // fallback
}

NodePtr AST::Derivative::complexLocalSimplify(OpPtr opNode) const
{
    auto& arguments = opNode->getArguments();

    if(arguments.size() != 2)
        throw std::runtime_error("Malformed differentiation! Cannot differentiate without both an operand and a variable to differentiate with respect to!");

    auto& function = arguments[0];
    auto withRespectTo = Casting::toVariable(arguments[1]);

    if(!withRespectTo)
        throw std::runtime_error("Malformed differentiation! Cannot differentiate to a non variable! (Does not support differentiating with respect to functions)");

    if(auto constant = Casting::toNumber(function))
        return NodeFactory::makeNumber(0); // d/dx(c) = 0, for all c which are real numbers
    
    if(auto variable = Casting::toVariable(function)) // d/dx(x) = 1, for all x
    {
        if(variable->identical(withRespectTo))
            return NodeFactory::makeNumber(1);
    }

    return opNode;
}

std::string AST::Derivative::toString(const std::vector<NodePtr> &args) const
{
    if(args.size() != 2)
        throw std::runtime_error("Malformed differentiation! ");

    return getName() + "(" + args[0]->toString()+","+args[1]->toString()+")";   
}

OpPtr AST::Derivative::buildInverseOf(OpPtr opToInvert) const
{
    if(opToInvert->getDefinition()->getName() != getName())
        return nullptr; // not the same operator

    auto& args = opToInvert->getArguments();
    if(args.size() != 2)
        return nullptr; // malformed derivative

    auto function = args[0];
    auto withRespectTo = args[1];

    // the inverse of a derivative is an integral, return a nullptr for now
    return nullptr;

}
}