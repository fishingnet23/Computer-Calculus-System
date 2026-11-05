#pragma once
#include "../Test.hpp"
#include "../../include/AlgebraSyntaxTree.hpp"


class InverseTest : public Test
{
    virtual TestResult performTest(const std::string& input, const std::string& expected) const override
    {
        AST::AlgebraExpression inputExpr(input);
        
        // Deliberately don't cannonize the original expression, 
        // as we want to test the inverse of the original expression itself

        if(expected == "NULL")
        {
            auto rootOp = AST::Casting::toOperator(inputExpr.getRoot());
            if(!rootOp)
                return {false,inputExpr.toString(),"Expected an operator at the root!"};
            auto inverseOp = rootOp->getDefinition()->buildInverseOf(rootOp);
            if(inverseOp)
                return {false,inputExpr.toString(),"Expected no inverse operator, but one was found!"};
            else
                return {true,"NULL","NULL"};
        }

        AST::AlgebraExpression expectedExpr(expected);
        expectedExpr.cannonize(); // cannonize expected

        auto rootOp = AST::Casting::toOperator(inputExpr.getRoot());
        if(!rootOp)
            return {false,inputExpr.toString(),"Expected an operator at the root!"};
        auto inverseOp = rootOp->getDefinition()->buildInverseOf(rootOp);
        if(!inverseOp)
            return {false,inputExpr.toString(),"Expected an inverse operator, but none was found!"};

        AST::AlgebraExpression inverseExpr(inverseOp);
        inverseExpr.cannonize(); // cannonize inverse
        // test inverse of inverse is original
        auto inverseOfInverseOp = inverseOp->getDefinition()->buildInverseOf(inverseOp);
        if(!inverseOfInverseOp)
            return {false,inverseExpr.toString(),"Expected an inverse operator of the inverse, but none was found!"};
        AST::AlgebraExpression inverseOfInverseExpr(inverseOfInverseOp);
        inverseOfInverseExpr.cannonize();
        if(!inverseOfInverseExpr.equals(inputExpr))
            return {false,inverseOfInverseExpr.toString(),"Expected the inverse of the inverse to be the original expression, but it was not!"};


        return {inverseExpr.equals(expectedExpr),inverseExpr.toString(),expectedExpr.toString()};
        
    }

    virtual std::string getName() const override {return "Inverse Test";}
};