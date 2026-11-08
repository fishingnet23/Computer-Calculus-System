#include "Utilities.hpp"

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

}
