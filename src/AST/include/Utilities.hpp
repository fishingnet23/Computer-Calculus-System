#pragma once
#include "Config.hpp"

namespace AST
{
    class Utilities
    {
    public:
        static void primeFactorize(InfInt n, std::vector<InfInt>& out);
        static void simplifyRoot(unsigned long long rootSize,const InfInt& inRadicand, InfInt& outCoeff, InfInt& outRadicand);
    };
}