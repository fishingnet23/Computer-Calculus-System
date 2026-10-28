#pragma once
#include "../Test.hpp"
#include "../../include/AlgebraSyntaxTree.hpp"


class CanonizeTest : public Test
{
    virtual TestResult performTest(const std::string& input, const std::string& expected) const override
    {
        AST::AlgebraExpression t1(input);
        AST::AlgebraExpression t2(expected);

        t1.cannonize();
        
        // normalize t2, it's reliant on the thing we are testing, but the key is that the two expressions represent the same thing.
        t2.cannonize();
 
        
        return {t1.equals(t2),t1.toString(),t2.toString()};
    }

    virtual std::string getName() const override {return "Cannonize Test";}
};