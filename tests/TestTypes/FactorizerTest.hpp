#pragma once
#include "../Test.hpp"
#include "../../include/AlgebraSyntaxTree.hpp"


class FactorizerTest : public Test
{
    virtual TestResult performTest(const std::string& input, const std::string& expected) const override
    {
        AST::AlgebraExpression t1(input);
        AST::AlgebraExpression t2(expected);

        t1.compress();
        t2.compress();
        // simplify, but crucially, don't expand

        t1.factorize();
        t2.factorize();
        // factorize both expressions

        return {t1.equals(t2),t1.toString(),t2.toString()};
    }

    virtual std::string getName() const override {return "Factorizer Test";}
};