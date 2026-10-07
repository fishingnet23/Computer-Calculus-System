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

std::shared_ptr<OperatorNode> OperatorNode::populateTreeFromVectorUsingSharedOperator(std::vector<NodePtr> &args, const std::string &op, const OperationRegistry &registry)
{
    if (args.empty()) {
        return nullptr;
    }
    if (args.size() == 1) {
        return std::dynamic_pointer_cast<OperatorNode>(args[0]);
    }

    auto root = std::make_shared<OperatorNode>(op, std::vector<NodePtr>{args[0], args[1]}, registry);
    for (size_t i = 2; i < args.size(); ++i) {
        root = std::make_shared<OperatorNode>(op, std::vector<NodePtr>{root, args[i]}, registry);
    }
    return root;

}

bool OperatorNode::equals(const OperatorNode *other) const
{
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

OperatorNode::OperatorNode(std::string op, std::vector<NodePtr> args, const OperationRegistry &registry): op(std::move(op)), procedureRegistry(registry) 
{
    arguments = std::move(args);
    for(const auto& arg : arguments) {
        arg->setFatherNode(this);
    }
    father = nullptr;
}
OperatorNode::OperatorNode(std::string op, std::vector<NodePtr> args, Node *parent, const OperationRegistry &registry)
: op(std::move(op)), procedureRegistry(registry) {
            arguments = std::move(args);
            father = parent;
    }

std::string OperatorNode::toString() const
{
    // Unary operator: op(arg)
    if (arguments.size() == 1) {
        return op + "(" + arguments[0]->toString() + ")";
    }
    // Binary operator: arg1 op arg2
    if (arguments.size() == 2) {
        bool r_associative = false;
        bool l_associative = false;
        auto pair = procedureRegistry.getRegistry().find(op);
        if (pair != procedureRegistry.getRegistry().end()) {
            const B_Operation* b_op = dynamic_cast<const B_Operation*>(pair->second.get());
            if (b_op) {
                r_associative = b_op->hasProperty(B_Operation::Property::RIGHT_ASSOCIATIVE);
                l_associative = b_op->hasProperty(B_Operation::Property::LEFT_ASSOCIATIVE);
            }
        }
        // if(l_associative && r_associative)
        //     return arguments[0]->toString() + " " + op + " " + arguments[1]->toString();
        return "(" + arguments[0]->toString() + " " + op + " " + arguments[1]->toString() + ")";
    }
    
    std::string str = "(";
    for (const auto& arg : arguments)
    {
        str += arg->toString() + " ";
    }
    str += ")";
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

NodePtr OperatorNode::simplifyStep()
{
    // Simplify all child nodes first
    std::vector<NodePtr> simplifiedArgs;
    for (const auto& child : arguments) {
        simplifiedArgs.push_back(child->simplifyStep());
    }

    bool allNumbers = true;
    for (const auto& arg : simplifiedArgs) {
        if (dynamic_cast<NumberNode*>(arg.get()) == nullptr) {
            allNumbers = false;
            break;
        }
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
    auto temp =  std::make_shared<OperatorNode>(op, simplifiedArgs, procedureRegistry);
    auto identity = procedureRegistry.simplifyWithIdentities(temp);

    const Operation* opPtr = procedureRegistry.getOperation(op);
    const B_Operation* b_op = opPtr? dynamic_cast<const B_Operation*>(opPtr) : nullptr;
    
    if(b_op == nullptr) {
        // not a binary operation, just return a new OperatorNode with the simplified arguments
        return std::make_shared<OperatorNode>(op, simplifiedArgs, procedureRegistry);
    }
    bool r_associative = b_op->hasProperty(B_Operation::Property::RIGHT_ASSOCIATIVE);
    bool l_associative = b_op->hasProperty(B_Operation::Property::LEFT_ASSOCIATIVE);
    bool commutative = b_op->hasProperty(B_Operation::Property::COMMUTATIVE);
    if(commutative) {
        std::sort(simplifiedArgs.begin(), simplifiedArgs.end(), [](const NodePtr& a, const NodePtr& b) {
            return a->toString() < b->toString();
        });
    }

    if(l_associative && r_associative) {
        auto flattenedArgs = flattenArgumentsWithSharedOperator(simplifiedArgs, op, procedureRegistry);
        std::sort(flattenedArgs.begin(), flattenedArgs.end(), [](const NodePtr& a, const NodePtr& b) {
            // const OperatorNode* opA = dynamic_cast<const OperatorNode*>(a.get());
            // const OperatorNode* opB = dynamic_cast<const OperatorNode*>(b.get());
            auto a_str = a->toString();
            auto b_str = b->toString();
            std::sort(a_str.begin(), a_str.end(), std::greater<char>());
            std::sort(b_str.begin(), b_str.end(), std::greater<char>());
            return a_str < b_str;
        });
        return populateTreeFromVectorUsingSharedOperator(flattenedArgs, op, procedureRegistry);
    }

    // final default case, return a new OperatorNode with the simplified arguments
    return std::make_shared<OperatorNode>(op, simplifiedArgs, procedureRegistry);
}

};
