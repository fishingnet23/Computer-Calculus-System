#include "../../include/Equation.hpp"
namespace AST
{   
Equation::Equation(const std::string& equationStr)
{
    Tokenizer t(equationStr);
    auto tokens = t.tokenize();
    auto equalPos = std::find_if(tokens.begin(), tokens.end(), [](const Token& t) { return t.type == TokenType::RELATION && t.value == "="; });
    if (equalPos == tokens.end()) {
        throw std::runtime_error("Equation must contain an '=' sign.");
    }
    std::vector<Token> lhsTokens(tokens.begin(), equalPos);
    std::vector<Token> rhsTokens(equalPos + 1, tokens.end());
    lhs = AlgebraExpression(lhsTokens);
    rhs = AlgebraExpression(rhsTokens);
}

void Equation::isolateVariable(const std::string& variableName)
{
    auto variableSubtree1 = lhs.findVariableNode(variableName);
    auto variableSubtree2 = rhs.findVariableNode(variableName);
    NodePtr variableSubtree = nullptr;
    if (variableSubtree1)
        variableSubtree = variableSubtree1;
    else if (variableSubtree2)
    {
        variableSubtree = variableSubtree2;
        swapSides(*this); // move the variable to the left side
    }
    else
        return; // variable not found
    
    isolateSubTree(variableSubtree);
}

void Equation::applyOperationToBothSides(const std::string& operation, const std::vector<NodePtr>& otherArgs, size_t treeNewIndex)
{

    // mutatble copies of the otherArgs for each side
    std::vector<NodePtr> lhsArgs = otherArgs;
    std::vector<NodePtr> rhsArgs = otherArgs;

    // insert the current side of the equation into the specified index of the arguments for both sides
    lhsArgs.insert(lhsArgs.begin() + treeNewIndex, lhs.getRoot());
    rhsArgs.insert(rhsArgs.begin() + treeNewIndex, rhs.getRoot());

    // create new operator nodes for both sides with the updated arguments
    NodePtr newLHS = NodeFactory::makeOperator(operation, lhsArgs);
    NodePtr newRHS = NodeFactory::makeOperator(operation, rhsArgs);

    // update the equation with the new sides
    lhs.setRoot(newLHS);
    rhs.setRoot(newRHS);
}

void Equation::isolateSubTree(NodePtr subtreeRoot)
{
    if (!subtreeRoot)
        return;

    // move all instances of the subtree to the left side
    auto leftRoot = lhs.getRoot();
    auto rightRoot = rhs.getRoot();




}


}
