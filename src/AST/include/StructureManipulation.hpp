#pragma once
#include "Operations.hpp"

namespace AST
{



// represents a meta variable that is a placeholder for an unknown operator
// structurally identical to an atomic variable node except it can't actually be evaluated!
// used for marking when a variable in a tree is actually a template for any kind of nested expression

using NodePredicate = bool (*)(NodePtr);

class PlaceholderNode : public VariableNode
{
    NodePredicate predicate;
public:

    PlaceholderNode():VariableNode(),predicate([](AST::NodePtr node){return true;}){}
    PlaceholderNode(bool (*predicate)(NodePtr)):VariableNode(),predicate(predicate){}
    explicit PlaceholderNode(const std::string&name, bool (*predicate)(NodePtr)):VariableNode(name),predicate(predicate){}
    explicit PlaceholderNode(const std::string&name):VariableNode(name),predicate([](AST::NodePtr node){return true;}){}

    PlaceholderNode(const PlaceholderNode& other):VariableNode(other),predicate(other.predicate){}
        
    virtual double evaluate(const Environment& env) const override{throw std::runtime_error("Abstract placeholder cannot be evaluated!");} 

    virtual PlaceholderNode* asPlaceholder() override {return this;}
    virtual const PlaceholderNode* asPlaceholder()const override {return this;}

    NodePredicate getPredicate()const{return predicate;}

    virtual NodePtr clone() const override {return std::make_shared<PlaceholderNode>(*this);} 

};


class StructureMatcher
{
    
public:
    static bool matchStructure(NodePtr subtree, OpPtr matchAgainst, std::map<std::string,NodePtr>& wildcardMap);
    static bool matchStructure(NodePtr subtree, OpPtr matchAgainst); // call if only care about success
    static OpPtr performRule(OpPtr subtree,OpPtr matchPrecondition,OpPtr ruleTemplate);
};
}