#include "StructureManipulation.hpp"
#include "NodeFactory.hpp"

namespace AST
{
bool StructureMatcher::matchStructure(NodePtr subtree, OpPtr matchAgainst, std::map<std::string, NodePtr> &wildcardMap)
{
    OpPtr op = Casting::toOperator(subtree);
    if(!op)
        return false; // no point in matching atomic nodes against an operator
    
    if(!op->sameOperator(matchAgainst)) // performs a surface level identifier check (for example, sin(x) and sin(x^2) pass this)
        return false; // if the outermost operators of both trees are not even the same, abort
    
    if(op->getArguments().size() != matchAgainst->getArguments().size()) // only compare n-ary with same argument count
        return false;
    
    for(size_t i=0; i <matchAgainst->getArguments().size();i++)
    {
        NodePtr arg = op->getArgument(i);
        NodePtr matchingArg = matchAgainst->getArgument(i);

        auto* wildcard = matchingArg->asPlaceholder();
        if(wildcard)
        {
            auto it = wildcardMap.find(wildcard->toString());
            if(it == wildcardMap.end()) // new wildcard
            {
                if(!(wildcard->getPredicate()(arg))) // checks if the argument meets the conditions of the wildcard
                // for example, if it's constants only, the predicate would check if the arg is castable to NumberPtr
                // if the argument we are trying to substitute doesn't pass the predicate, don't match
                    return false;
                wildcardMap[wildcard->toString()] = arg;
                continue; // assume the argument is the intended inner expression, if not, a contradiction will be found later
            }
            auto cachedNode = wildcardMap[wildcard->toString()];
            if(!cachedNode->identical(arg))
                return false; // contradiction, this means the true shape is different than expected
            continue;
        }
        
        OpPtr matchingOp = Casting::toOperator(matchingArg);

        if(matchingOp)
        {
            if(!matchStructure(arg,matchingOp,wildcardMap)) // recursively check nesting
                return false;
        }
        else if(!matchingArg->identical(arg)) // atomic comparison
            return false;
    }

    return true; // successfully matched with no contradictions
}

bool StructureMatcher::matchStructure(NodePtr subtree, OpPtr matchAgainst)
{
    std::map<std::string,NodePtr> map; // anchor to stack
    return matchStructure(subtree,matchAgainst,map);
}

OpPtr AST::StructureMatcher::performRule(OpPtr subtree, OpPtr matchPrecondition, OpPtr ruleTemplate)
{
    std::map<std::string,NodePtr> map;

    bool success = matchStructure(subtree,matchPrecondition,map);

    if(!success)
        return subtree; // return the unmodified tree

    // the structure has been matched, substitution is legal
    return NodeFactory::substitute(ruleTemplate,map);
}



}