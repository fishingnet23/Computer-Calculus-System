#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
namespace AST
{
namespace mp = boost::multiprecision;
using InfRational = mp::cpp_rational;
using InfInt = mp::cpp_int;
using InfFloat = boost::multiprecision::number<
    boost::multiprecision::cpp_dec_float<100> // not infinite sadly, but massive
>;
class Config
{
public:
    static const bool DEBUG_MODE = false;
    static constexpr int MAX_FIXED_POINT_ITERATIONS = 50;
    static constexpr double EPSILON = 1e-12;
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double E = 2.71828182845904523536;
};


}