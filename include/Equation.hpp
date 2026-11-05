#pragma once
#include "AlgebraSyntaxTree.hpp"
namespace AST
{
class Equation {
    AlgebraExpression lhs;
    AlgebraExpression rhs;
public:
    Equation(const AlgebraExpression& lhs, const AlgebraExpression& rhs) : lhs(lhs), rhs(rhs) {}
    Equation(const std::string& lhs, const std::string& rhs) : lhs(AlgebraExpression(lhs)), rhs(AlgebraExpression(rhs)) {}
    Equation(const std::vector<Token>& lhs, const std::vector<Token>& rhs) : lhs(AlgebraExpression(lhs   )), rhs(AlgebraExpression(rhs)) {}
    explicit Equation(const std::string& equationStr);
    
    const AlgebraExpression& getLHS() const { return lhs; }
    const AlgebraExpression& getRHS() const { return rhs; }
    AlgebraExpression& getLHS() { return lhs; }
    AlgebraExpression& getRHS() { return rhs; }
    AlgebraExpression getLHSCopy() const { return lhs; }
    AlgebraExpression getRHSCopy() const { return rhs; }

    void setLHS(const AlgebraExpression& newLHS) { lhs = newLHS; }
    void setRHS(const AlgebraExpression& newRHS) { rhs = newRHS; }

    std::string toString() const {return lhs.toString() + " = " + rhs.toString();}

    static void swapSides(Equation& eq){std::swap(eq.lhs, eq.rhs);}


    // Applies the specified operation to both sides of the equation.
    // The operation is specified by its name and other arguments.
    // the treeNewIndex parameter is used to specify which argument of the operation should be replaced by the current side of the equation.
    void applyOperationToBothSides(const std::string& operation, const std::vector<NodePtr>& otherArgs, size_t treeNewIndex = 0);

    // wrapper for the function below, just isolates the variable with the given name,
    // if it exists in the equation.
    void isolateVariable(const std::string& variableName);
    
    // This function locates all instances of the specified subtree within the equation
    // Moves all the instances to the left side, and performs inverse operations on the right side to isolate the left hand side.
    // Note that it doesn't fully isolate the subtree, it just moves all instances to the left side and applies inverse operations to the right side.
    // for example, if the equation is "x^2 +3x +32 = 21 -3x" and the subtree is "3x",
    // it will isolate all instances of "3x" to the left side,
    // resulting in "6x = -x^2-53".
    // If the sub tree was x, it would put all x expressions to the left, and all constants to the right
    // resulting in "x^2 + 5x = -53".
    void isolateSubTree(NodePtr subtreeRoot);
};
}