#pragma once
#include "../src/AST/Parsing/Parser.hpp"

namespace AST
{
class AlgebraExpression
{
    NodePtr root;
    NodePtr parseTokens(const std::vector<Token>& tokens);
public:
    explicit AlgebraExpression(NodePtr root) :root(root) {}
    AlgebraExpression() :root(nullptr) {}
    explicit AlgebraExpression(const std::vector<Token>& tokens);
    explicit AlgebraExpression(const std::string& src);

    void buildFrom(const std::string& src);
    void buildFrom(const std::vector<Token>& tokens){root = parseTokens(tokens);}

    NodePtr getRoot() const { return root; }
    void setRoot(NodePtr newRoot) { root = newRoot;}

    std::string toString() const
    {
        if (root)
            return root->toString();
        else
            return "EMPTY TREE";
    }

    std::string postfixNotation()
    {
        if(root)
            return root->getPostFix();
        return "EMPTY TREE";
    }

    std::string prefixNotation()
    {
        if(root)
            return root->getPreFix();
        return "EMPTY TREE";
    }

    AlgebraExpression substituteEnvironment(const Environment& env) const
    {
        NodePtr temp = NodeFactory::substitute(root,env);
        AlgebraExpression tempExpr(temp);
        tempExpr.cannonize();

        int count = 0;
        while(count < Config::MAX_FIXED_POINT_ITERATIONS)
        {
            tempExpr.setRoot(NodeFactory::substitute(tempExpr.getRoot(),env));
            tempExpr.cannonize();
            
            if(tempExpr.getRoot() == temp)
                break;
            temp = tempExpr.getRoot();
            count++;
        }

        return tempExpr;
    }

    InfFloat approximate() const
    {
        if(auto op = SoftCasting::toOperator(root))
        {
            return OperatorNode::approximate(op);
        }
        else if(auto n = SoftCasting::toRawNumber(root))
            return n->getValue();
        else
            return 0; // default
    }

    void compress()
    {
        if(!root)
            return;
        
        auto op = SoftCasting::toOperator(root);
        if(!op)
            return;
        NodePtr newRoot = OperatorNode::simplify(op);
        root = std::move(newRoot);
    }

    void expand()
    {
        if(!root)
            return;
        auto op = SoftCasting::toOperator(root);
        if(!op)
            return;
        NodePtr newRoot = OperatorNode::expand(op);
        root = std::move(newRoot);
    }

    void unify()
    {
       if(!root)
            return;
        auto op = SoftCasting::toOperator(root);
        if(!op)
            return;
        NodePtr newRoot = OperatorNode::unify(op);
        root = std::move(newRoot);
    }

    void cannonize()
    {
        int count = 0;
        NodePtr rootBefore;
        do
        {
            rootBefore = root;
            expand();
            count++;
        }while(root && root != rootBefore && count < Config::MAX_FIXED_POINT_ITERATIONS); // pointer check
        count = 0;
        rootBefore = root;
        do
        {
            rootBefore = root;
            compress();
            count++;
        }while(root && root != rootBefore && count < Config::MAX_FIXED_POINT_ITERATIONS); // pointer check
    }

    bool equals(const AlgebraExpression& other)
    {
        if(!root || !other.root)
            return false;
        return root == other.root;
    }

    NodePtr findVariableNode(const std::string& variableName);
    
};
}