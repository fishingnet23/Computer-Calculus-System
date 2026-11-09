#pragma once
#include <unordered_set>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
namespace AST
{
namespace mp = boost::multiprecision;
using InfRational = mp::cpp_rational;
using InfInt = mp::cpp_int;


constexpr int FLOAT_PRECISION = 128; // not infinite sadly, but massive
using InfFloat = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<FLOAT_PRECISION>>;
class Config
{
public:
    static const bool DEBUG_MODE = false;
    static constexpr int MAX_FIXED_POINT_ITERATIONS = 50;
    static constexpr double EPSILON = 1e-15; // decently large for the framework
    static constexpr int FLOAT_PRECISION = FLOAT_PRECISION; 

    // 100 digits of accuracy

    static auto& getConstantMap() {
        static std::unordered_map<std::string,InfFloat> instance = {
            {"pi",InfFloat("3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679")},
            {"e",  InfFloat("2.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274")},
        };

        return instance;
    }

    static auto& getDummySymbols()
    {
        static std::unordered_set<std::string> instance = {"i"};
        return instance;
    }

    static bool isConstant(const std::string& str)
    {
        auto it = getConstantMap().find(str);
        return it!=getConstantMap().end();
    }
};


}