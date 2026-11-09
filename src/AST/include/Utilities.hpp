#pragma once
#include "Config.hpp"
#include "../Structure/Nodes.hpp"
namespace AST
{
    class Utilities
    {
    public:
        static void primeFactorize(InfInt n, std::vector<InfInt>& out);
        static void simplifyRoot(unsigned long long rootSize,const InfInt& inRadicand, InfInt& outCoeff, InfInt& outRadicand);

        static std::unordered_map<NodePtr,NodePtr> extractFactors(const NodePtr& node);
        // find the symbolic gcd of the arguments passed in
        // returns the gcd directly 
        // the returned NodePtr a is the factor such that for any operator f that distributes over multiplication,
        // f(a*b,a*c,a*d,...) = a*f(b,c,d,...)
        // prime example: 5x^2 - 5x = 5x(x-1) so the function returns 5x and outputs the factored arguments
        // specifically the greatest factor
        // defaults to returning a NodePtr to the multiplicative identity 1 if there is no gcd better than 1
        static NodePtr symbolicGcd(const std::vector<NodePtr>& arguments, std::vector<NodePtr>& factorsLeft);
    };
}