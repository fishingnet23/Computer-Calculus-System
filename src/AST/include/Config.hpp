#pragma once
namespace AST
{
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