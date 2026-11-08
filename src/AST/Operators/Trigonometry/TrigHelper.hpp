#pragma once
#include "../../Structure/OperatorDefinitions.hpp" 

namespace AST
{

    struct PiRational
    {
        InfInt numerator = 0;
        InfInt denominator = 1;
        bool isValid = false;
    };

    // precondition: the input NodePtr must be in canonical form
    static void extractPiAngle(NodePtr in, PiRational& out)
    {
        out.isValid = false; // reset state
        auto symbol = SoftCasting::toSymbol(in);

        if(auto rat = SoftCasting::toNumber(in); rat && rat->getValue() == 0)
        {
            out = {0,1,true};
            return;
        }

        if(symbol && symbol->getName() == "pi")
        {
            out = {1, 1, true}; // just pi
            return;
        }

        if(auto op = SoftCasting::toOperator(in); op && op->getName() == "*")
        {
            // a pi rational must be of the form (a/b)*pi, where a/b is a rational node
            if(op->getArguments().size() > 2)
                return;

            auto coeff = SoftCasting::toRational(op->getArgument(0));
            if(!coeff) // doesn't have a coeff
                return;
            
            auto pi = SoftCasting::toSymbol(op->getArgument(1));
            if(!pi || pi->getName() != "pi")
                return;

            auto frac = coeff->getFraction(); 
            
            InfInt N = numerator(frac);
            InfInt D = denominator(frac);
            
            InfInt period = 2 * D;
            InfInt reducedN = N % period;
            
            // force range [0,2pi]
            if (reducedN < 0) {
                reducedN += period;
            }
            
            out = {reducedN, D, true};
            return;
        }
    }
}