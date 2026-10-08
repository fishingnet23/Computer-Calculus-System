#include "TreeNodes.h"
namespace AST
{
std::vector<NodePtr> OperatorNode::flattenArgumentsWithSharedOperator(const std::vector<NodePtr> &args, const std::string &op, const OperationRegistry &registry)
    {
    std::vector<NodePtr> flattenedArgs;
    for (const auto& arg : args) {
        if (auto opNode = std::dynamic_pointer_cast<OperatorNode>(arg)) {
            if (opNode->op == op) {
                auto nestedFlattened = opNode->flattenArgumentsWithSharedOperator(opNode->arguments,op,registry);
                flattenedArgs.insert(flattenedArgs.end(), nestedFlattened.begin(), nestedFlattened.end());
            } else {
                flattenedArgs.push_back(arg);
            }
        } else {
            flattenedArgs.push_back(arg);
        }
    }
    return flattenedArgs;
}

NodePtr OperatorNode::populateTreeFromVectorUsingSharedOperator(
    std::vector<NodePtr>& args,
    const std::string& op,
    const OperationRegistry& registry)
{
    if (args.empty())
        return nullptr;

    if (args.size() == 1)
        return args[0];

    NodePtr root = std::make_shared<OperatorNode>(
        op,
        std::vector<NodePtr>{args[0], args[1]},
        registry
    );

    for (size_t i = 2; i < args.size(); ++i)
    {
        root = std::make_shared<OperatorNode>(
            op,
            std::vector<NodePtr>{root, args[i]},
            registry
        );
    }

    return root;
}

OperatorNode::OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry &registry): op(std::move(op)), procedureRegistry(registry), Symbol(1.0,1.0)
{
    arguments.reserve(args.size());
    for(const auto& arg:args)
        arguments.push_back(arg->clone());
    type = NodeType::Operator;
}


OperatorNode::OperatorNode(const OperatorNode &other):op(other.op),procedureRegistry(other.procedureRegistry),Symbol(other)
{
    arguments.reserve(other.arguments.size());
    for(const auto& arg:other.arguments)
        arguments.push_back(arg->clone());
    type = NodeType::Operator;
}

std::string OperatorNode::toString() const
{
    
    std::string str = "";
    
    if(coeffecient != 1.0 && coeffecient != -1.0)
        str += std::to_string(coeffecient);
    if(coeffecient == -1.0)
        str += "-";

    // Unary operator: op(arg)
    if (arguments.size() == 1) {
        str += op + "(" + arguments[0]->toString() + ")";
    }
    // Binary operator: arg1 op arg2
    else if (arguments.size() == 2) {
        bool r_associative = false;
        bool l_associative = false;
        auto pair = procedureRegistry.getRegistry().find(op);
        if (pair != procedureRegistry.getRegistry().end()) {
            const B_OpDefinition* b_op = dynamic_cast<const B_OpDefinition*>(pair->second.get());
            if (b_op) {
                r_associative = b_op->hasProperty(B_OpDefinition::Property::RIGHT_ASSOCIATIVE);
                l_associative = b_op->hasProperty(B_OpDefinition::Property::LEFT_ASSOCIATIVE);
            }
        }
        if(l_associative && r_associative)
            str+= arguments[0]->toString() + " " + op + " " + arguments[1]->toString();
        else
            str+="(" + arguments[0]->toString() + " " + op + " " + arguments[1]->toString() + ")";
    }
    else 
    {
        for (const auto& arg : arguments)
        {
            str += arg->toString() + " ";
        }
        str += ")";
    }

    if (degree != 1.0)
        str += "^" + std::to_string(degree);

    return str;
}

double OperatorNode::evaluate(const Environment &env) const
{
    // Evaluate all child nodes first, this ensures nested operations and functions get evaluated first
    std::vector<double> evaluatedArgs;
    for (const auto& child : arguments) {
        evaluatedArgs.push_back(child->evaluate(env));
    }

    // Look up in the registry and calculate the final result
    return procedureRegistry.execute(op, evaluatedArgs);
}

NodePtr OperatorNode::simplifyStep() const
{
    // Simplify all child nodes first
    bool allNumbers = true;
    std::vector<NodePtr> simplifiedArgs;
    for (const auto& child : arguments) {

        if (const auto& symbol = child->asSymbol())
        {
            simplifiedArgs.push_back(symbol->simplifyStep());
            allNumbers = false;
        }
        else
            simplifiedArgs.push_back(child);
    }

    if (allNumbers) {
        Environment dummyEnv; // no variables needed for pure number evaluation
        std::vector<double> argsValues;
        for (const auto& arg : simplifiedArgs) {
            argsValues.push_back(arg->evaluate(dummyEnv));
        }
        double result = procedureRegistry.execute(op, argsValues);
        return std::make_shared<NumberNode>(result);
    
    }

    // try identities
    auto backupClone = this->clone();
    auto tempOp = static_cast<OperatorNode*>(backupClone.get());
    tempOp->setArguments(simplifiedArgs);
    auto identity = procedureRegistry.simplifyWithIdentities(backupClone);

    // the identity changed the structure of the node
    if(!backupClone->equals(identity.get()))
        return identity;

    const OpDefiniton* opPtr = procedureRegistry.getOperation(op);
    const B_OpDefinition* b_op = opPtr? dynamic_cast<const B_OpDefinition*>(opPtr) : nullptr;
    
    if(b_op == nullptr) {
        // not a binary operation, just return a new OperatorNode with the simplified arguments
        return backupClone;
    }
    bool r_associative = b_op->hasProperty(B_OpDefinition::Property::RIGHT_ASSOCIATIVE);
    bool l_associative = b_op->hasProperty(B_OpDefinition::Property::LEFT_ASSOCIATIVE);
    bool commutative = b_op->hasProperty(B_OpDefinition::Property::COMMUTATIVE);
    

    if(l_associative && r_associative) {
        auto flattenedArgs = flattenArgumentsWithSharedOperator(simplifiedArgs, op, procedureRegistry);
        if(commutative)
            std::sort(flattenedArgs.begin(),flattenedArgs.end(),Node::canonicalLess);
        auto groups = groupLikeTerms(flattenedArgs);

        std::vector<NodePtr> reducedGroups;

        for (const auto& group : groups)
            reducedGroups.push_back(reduceGroup(group,op,procedureRegistry));

        return populateTreeFromVectorUsingSharedOperator(reducedGroups,op,procedureRegistry);
    }

    if(commutative) {
        std::sort(simplifiedArgs.begin(),simplifiedArgs.end(),Node::canonicalLess);
    }

    // final default case, return a new OperatorNode with the simplified arguments
    return backupClone;
}

bool OperatorNode::equals(const Node *otherNode) const
{
    if(otherNode->type != NodeType::Operator)
        return false;
    const auto* other = static_cast<const OperatorNode*>(otherNode);

    if(op!=other->op || arguments.size()!=other->arguments.size())
        return false;
    
    bool equal = true;

    for(size_t i=0;i<arguments.size();i++)
    {
        if(!arguments[i].get()->equals(other->arguments[i].get()))
        {
            equal = false;
            break;
        }
    }

    return equal;
}

bool OperatorNode::equalBases(const Symbol *otherNode) const
{
    if(otherNode->type != NodeType::Operator)
        return false;
    const auto* other = static_cast<const OperatorNode*>(otherNode);

    if(op!=other->op || arguments.size()!=other->arguments.size())
        return false;
    
    bool congruent = true;

    for(size_t i=0;i<arguments.size();i++)
    {
        auto symbolArg = arguments[i]->asSymbol();
        auto symbolOther = other->arguments[i]->asSymbol();
        if (!symbolArg != !symbolOther) {
            congruent = false;
            break;
        }

        if (symbolArg) {
            if (!symbolArg->equalBases(symbolOther)) {
                congruent = false;
                break;
            }
        } else {
            if (!arguments[i]->equals(other->arguments[i].get())) {
                congruent = false;
                break;
            }
        }

    }

    return congruent;
}

std::vector<std::vector<NodePtr>> OperatorNode::groupLikeTerms(const std::vector<NodePtr> &args)
{
    std::vector<std::vector<NodePtr>> groups;

    for (const auto& arg : args)
    {
        bool added = false;

        for (auto& group : groups)
        {
            const Symbol* a = arg->asSymbol();
            const Symbol* b = group[0]->asSymbol();

            if (a && b &&
                a->equalBases(b) &&
                a->getDegree() == b->getDegree())
            {
                group.push_back(arg);
                added = true;
                break;
            }

            // Same constant category
            if (dynamic_cast<const NumberNode*>(arg.get()) &&
                dynamic_cast<const NumberNode*>(group[0].get()))
            {
                group.push_back(arg);
                added = true;
                break;
            }
        }

        if (!added)
            groups.push_back({arg});
    }

    return groups;
}
NodePtr OperatorNode::reduceGroup(
    const std::vector<NodePtr>& group,
    const std::string& op,
    const OperationRegistry& registry)
{
    if (group.empty())
        return nullptr;

    NodePtr result = group[0];

    for (size_t i = 1; i < group.size(); ++i)
    {
        auto pair = std::make_shared<OperatorNode>(
            op,
            std::vector<NodePtr>{result, group[i]},
            registry
        );

        result = registry.simplifyWithIdentities(pair);
    }

    return result;
}
};


