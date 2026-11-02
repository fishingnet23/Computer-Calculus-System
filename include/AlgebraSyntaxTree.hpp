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

    NumberPtr evaluate(const Environment& env) const
    {
        if (auto symR = Casting::toSymbol(root))
            return symR->evaluate(env);
        else if(auto n = Casting::toNumber(root))
            return n;
        else
            throw std::runtime_error("Unevaluateable root!");
    }

    void compress()
    {
        if(!root)
            return;
        
        auto op = Casting::toOperator(root);
        if(!op)
            return;
        NodePtr newRoot = OperatorNode::simplify(op);
        root = std::move(newRoot);
    }

    void expand()
    {
        if(!root)
            return;
        auto op = Casting::toOperator(root);
        if(!op)
            return;
        NodePtr newRoot = OperatorNode::expand(op);
        root = std::move(newRoot);
    }

    void unify()
    {
       if(!root)
            return;
        auto op = Casting::toOperator(root);
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
            compress();
            count++;
        }while(root && root != rootBefore && count < Config::MAX_FIXED_POINT_ITERATIONS); // pointer check
    }

    bool equals(const AlgebraExpression& other)
    {
        if(!root || !other.root)
            return false;
        return root->identical(other.root);
    }
};
}