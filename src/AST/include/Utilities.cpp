#include "Utilities.hpp"
#include "NodeFactory.hpp"
namespace AST
{

// trial division algorithm
void Utilities::primeFactorize(InfInt n, std::vector<InfInt>& out) {
    if (n <= 1) return;

    // strip out evens for tiny optimization
    while (n % 2 == 0) {
        out.push_back(2);
        n /= 2;
    }

    // check numbers up to sqrt(n), multiplication is a binary operation, and so for any pair of two factors, at least one is smaller than sqrt(n)
    // this can be trivially proven by contradiction
    for (InfInt i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            out.push_back(i);
            n /= i;
        }
    }

    // 3. If n is still greater than 1, whatever is left over MUST be prime!
    if (n > 1) {
        out.push_back(n);
    }
}

void Utilities::simplifyRoot(unsigned long long rootSize, const InfInt& inRadicand, InfInt &outCoeff, InfInt &outRadicand)
{
    outCoeff = 1;
    outRadicand = 1;

    // Edge case: root of 0 is always 0
    if (inRadicand == 0) {
        outCoeff = 0;
        outRadicand = 0;
        return;
    }

    std::vector<InfInt> factors;
    primeFactorize(inRadicand, factors);

    // Group identical primes and extract roots
    size_t i = 0;
    while (i < factors.size()) {
        InfInt currentPrime = factors[i];
        unsigned long long count = 0;

        while (i < factors.size() && factors[i] == currentPrime) {
            count++;
            i++;
        }

        unsigned long long pulledOut = count / rootSize;
        unsigned long long leftInside = count % rootSize;

        for (unsigned long long k = 0; k < pulledOut; ++k) {
            outCoeff *= currentPrime;
        }

        for (unsigned long long k = 0; k < leftInside; ++k) {
            outRadicand *= currentPrime;
        }
    }
}

struct FactoredView
{
    NodePtr base;
    NodePtr exponent;
};

FactoredView normalizeFactor(const NodePtr& node) {
    if (auto op = SoftCasting::toOperator(node); op && op->getName() == "^") { 
        return { op->getArgument(0), op->getArgument(1)};
    }
    
    return { node, NodeFactory::makeNumber(1) };
}

std::unordered_map<NodePtr,NodePtr> Utilities::extractFactors(const NodePtr& node)
{
    auto op = SoftCasting::toOperator(node);
    if(!op || op->getName() != "*")
    {
        auto normal = normalizeFactor(node);
        return {{normal.base, normal.exponent}}; 
    }

    auto simplified = OperatorNode::localSimplify(op); // forces numeric coeff to be collapsed
    OpPtr normalizedProduct = SoftCasting::toOperator(simplified); 

    if(!normalizedProduct)
    {   
        auto normal = normalizeFactor(simplified);
        return {{normal.base,normal.exponent}};
    }


    std::unordered_map<NodePtr,NodePtr> res;
    
    for(const auto& arg : normalizedProduct->getArguments())
    {
        auto normal = normalizeFactor(arg);
        
        auto existing = res.find(normal.base);
        if (existing != res.end()) {
            OpPtr combinedExp = NodeFactory::makeOperator("+", {existing->second, normal.exponent}); 
            res[normal.base] = OperatorNode::simplify(combinedExp);// handle unsimplified edge cases like x^x*x^3 so it can be cleanly handled as x^(x+3)
        } else {
            res[normal.base] = normal.exponent;
        }
    }

    return res;
}

NodePtr Utilities::symbolicGcd(const std::vector<NodePtr> &arguments, std::vector<NodePtr> &factorsLeft)
{
    // ensure there is something to factor
    auto one = NodeFactory::makeNumber(1);

    if(arguments.empty())
    {
        factorsLeft.clear();
        return one;
    }
    if(arguments.size() == 1)
    {
        factorsLeft = {one};
        return arguments[0];
    }

     // extract the baseline factors from the first argument
    std::unordered_map<NodePtr,NodePtr> factors  = extractFactors(arguments[0]); // factor -> highest common degree

    // extract baseline rational coeff
    InfRational commonCoeff(1);
    for (auto it = factors.begin(); it != factors.end(); )
    {
        if (auto rational = SoftCasting::toRational(it->first); rational && it->second == one) {
            commonCoeff *= rational->getFraction();
            it = factors.erase(it); // remove it from symbolic factors
        } else {
            ++it;
        }
    }

    // determine the gcd, the greatest common divisors

    for(size_t i=1;i<arguments.size();i++)
    {
        std::unordered_map<NodePtr,NodePtr> innerFactors = extractFactors(arguments[i]);

        InfRational currentCoeff(1);
        for (auto it = innerFactors.begin(); it != innerFactors.end(); )
        {
            if (auto rational = SoftCasting::toRational(it->first); rational && it->second == one) {
                currentCoeff *= rational->getFraction();
                it = innerFactors.erase(it); // remove it from symbolic factors
            } else {
                ++it;
            }
        }

        commonCoeff = InfRational(
            gcd(numerator(commonCoeff), numerator(currentCoeff)), 
            lcm(denominator(commonCoeff), denominator(currentCoeff))
        ); // accumulate into the commonCoeff through the gcd formula for rational numbers

        // now process purely symbolic terms safely
        for (auto it = factors.begin(); it != factors.end(); )
        {
            const NodePtr& base = it->first;
            const NodePtr& currentExp = it->second;

            auto match = innerFactors.find(base);

            if(match == innerFactors.end())
            { // this factor is missing, it must be deleted from the shared factor map
                it = factors.erase(it);
                continue;
            }

            const NodePtr& matchingExp = match->second;

            NodePtr delta = OperatorNode::canonicalize(NodeFactory::makeOperator("-",{currentExp,matchingExp}));

            if(NumberPtr deltaNumber = SoftCasting::toNumber(delta))
            {
                if(deltaNumber->getValue() > 0) // the exponent present in the factor map is too large
                    it->second = matchingExp;
                // in other cases, the currentExp remains since it is smaller or equal
                ++it; // next factor
            }
            else
                it = factors.erase(it); // if the difference can't be ordered, just don't try to claim that factor, pop it out

        }
        if(factors.empty() && commonCoeff == 1) { // nothing left to compare, there are no common factors
            factorsLeft = arguments;
            return one;
        }
    }

    // rebuild factors left after dividing the gcd out
    factorsLeft.clear();
    factorsLeft.reserve(arguments.size());

    for (const auto& arg : arguments)
    {
        std::unordered_map<NodePtr, NodePtr> argFactors = extractFactors(arg);
        

        InfRational myCoeff(1);
        for (auto it = argFactors.begin(); it != argFactors.end(); )
        {
            if (auto rational = SoftCasting::toRational(it->first); rational && it->second == one) {
                myCoeff *= rational->getFraction();
                it = argFactors.erase(it);
            } else {
                ++it;
            }
        }
        InfRational remainingCoeff = myCoeff / commonCoeff;
        
        std::vector<NodePtr> dividedTerms;
        if(remainingCoeff != 1)
            dividedTerms.push_back(NodeFactory::makeNumber(remainingCoeff));
        
        // subtract the symbolic gcd's exponent from the existing exponent to find the quotient
        for (const auto& [gcdBase, gcdExp] : factors)
        {
            auto match = argFactors.find(gcdBase);
            // Guaranteed to find a match because gcdBase is a common divisor
            
            NodePtr remainingExp = OperatorNode::canonicalize(NodeFactory::makeOperator("-", {match->second, gcdExp}));
            
            if (NumberPtr expNum = SoftCasting::toNumber(remainingExp); expNum && expNum->getValue() == 0) {
                // exponent became zero, it's unneeded now
                argFactors.erase(match);
            } else {
                // update to the quotient's exponent
                match->second = remainingExp;
            }
        }

        // rebuild the divided argument back into an expression
        for (auto& [base, exp] : argFactors) {
            if (NumberPtr expNum = SoftCasting::toNumber(exp); expNum && expNum->getValue() == 1) {
                dividedTerms.push_back(std::move(base)); // if the exponent is 1, no need for a power node
            } else {
                dividedTerms.push_back(NodeFactory::makeOperator("^", {std::move(base), std::move(exp)}));
            }
        }

        if (dividedTerms.empty()) {
            factorsLeft.push_back(one); // base case if there's nothing left after division, leave a 1 (like xy + x -> x(y+1))
        } else {
            factorsLeft.push_back(OperatorNode::canonicalize(NodeFactory::makeOperator("*", dividedTerms)));
        }
    }


    // rebuild the shared factors for the gcd itself
    std::vector<NodePtr> sharedFactors;
    sharedFactors.reserve(factors.size() + 1);

    if (commonCoeff != 1) {
        sharedFactors.push_back(NodeFactory::makeNumber(commonCoeff));
    }

    for (auto& [base, exp] : factors) 
    {
        if (NumberPtr expNum = SoftCasting::toNumber(exp); expNum && expNum->getValue() == 1) {
            sharedFactors.push_back(std::move(base));
        } else {
            sharedFactors.push_back(NodeFactory::makeOperator("^", {std::move(base), std::move(exp)}));
        }
    }
   
    NodePtr gcd = OperatorNode::simplify(NodeFactory::makeOperator("*",sharedFactors));
    return gcd;
}

}
