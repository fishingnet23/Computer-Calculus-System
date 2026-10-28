#include <iostream>
#include "TestTypes/CannonizeTest.hpp"
#include <sstream>


// good test cases
// (x^2 - x^2) + 3x^2(x^0 + 4) - 15x^2
// 1 / 2x / 3x^2 / (1 / 4x)
// (2x^2)^3^2 - 512x^18
// sin(x) / cos(x) -x ^ 0 * sin(x) / cos(x)
// x - -2x^2 - (3x^2 - -x)

int main() 
{
    CanonizeTest cannonize;
    
    // cannonize.hideDescriptions();
    cannonize.showDescriptions(); // uncomment to enable descriptions

    cannonize.test("----x","x","Chain Negation");
    cannonize.test("-----x","-x","Chained Negation");
    cannonize.test("-(-x)", "x", "Double negation test");
    cannonize.test("0-x", "-x", "Subtraction from zero test");
    cannonize.test("x-0", "x", "Subtraction of zero test");
    cannonize.test("x-(-y)", "x+y", "Subtract negative test");

    cannonize.test("x*0", "0", "Zero collapse test");
    cannonize.test("0*x", "0", "Zero collapse test");
    cannonize.test("x*x*y*x*0", "0", "Zero collapse test");
    cannonize.test("(y*x)^2*0", "0", "Zero collapse test");
    cannonize.test("x+0","x","Addition identity test");
    cannonize.test("0+x","x","Addition identity test");
    cannonize.test("x*1" ,"x", "1 multiplication test");
    cannonize.test("1*x", "x", "1 multiplication test");
    cannonize.test("x^1", "x", "Degree 1 identity test");
    cannonize.test("1^x", "1", "Base 1 exponent test");
    cannonize.test("x^a * x^b", "x^(a+b)", "Exponent multiplication rule");
    cannonize.test("(x^a)^b", "x^(a*b)", "Power of a power rule");

    cannonize.test("0/x", "0", "Zero numerator division test");
    cannonize.test("x/1", "x", "Division by 1 identity test");
    cannonize.test("(-x)/y", "-(x/y)", "Negative quotient test");
    cannonize.test("x/x", "1", "Self division identity");
    cannonize.test("(x*y)/x", "y", "Numerator-denominator cancellation");
    cannonize.test("x^-1 * x", "1", "Negative exponent cancellation");
    cannonize.test("1/(1/x)", "x", "Reciprocal of reciprocal");

    cannonize.test("x^0","1","degree 0 test");
    cannonize.test("5x^0","5","degree 0 test");
    cannonize.test("0x^0","0","degree 0 and coeffecient 0 test");
    cannonize.test("(x^2)^3", "x^6", "Power of a power evaluation");
    cannonize.test("(2x)^3", "8x^3", "Power of product distribution");

    cannonize.test("(x+1)(x+2)","x^2+3x+2","Polynomial expansion test");
    cannonize.test("(x+1)(x+1)","x^2+2x+1","Polynomial expansion test");
    cannonize.test("(x+3)^2","x^2+6x+9","Polynomial expansion test");
    cannonize.test("(x+3)^3*(x+2)^2", "x^5+13x^4+67x^3+171x^2+216x+108", "Large polynomial expansion test");
    cannonize.test("(x-1)(x+1)", "x^2-1", "Difference of squares expansion");
    cannonize.test("(x-y)(x+y)", "x^2-y^2", "Difference of squares expansion with variables");
    cannonize.test("(x-1)^3", "x^3-3x^2+3x-1", "Binomial cube expansion");
    cannonize.test("a*(b+c)", "a*b+a*c", "Distributive property expansion");
    cannonize.test("2*(x+3)", "2x+6", "Scalar distribution");
    cannonize.test("-(x+y)", "-x-y", "Negation distribution");
    
    cannonize.test("a-a","0","addition inverse identity");
    cannonize.test("a + (-a)","0","addition inverse identity");

    cannonize.test("a/a","1","Multiplication inverse identity");
    cannonize.test("a * (a^-1)","1","Multiplication inverse identity");

    cannonize.test("log(1,b)", "0", "Logarithm of 1 test");
    cannonize.test("log(b,b)", "1", "Logarithm of base test");
    cannonize.test("ln(1)", "0", "Natural log of 1 test");
    cannonize.test("ln(e)", "1", "Natural log of e test");
    cannonize.test("log(2^x,2)", "x","Logarithm/Exponent inverse identity");
    cannonize.test("2^log(x,2)", "x","Logarithm/Exponent inverse identity");

    cannonize.test("log(2^x,3)", "x*log(2,3)","Pull down property");
    cannonize.test("log(3^x,2)", "x*log(3,2)","Pull down property");

    cannonize.test("(x+2)^log(5x,x+2)", "5x","Logarithm/Exponent inverse identity for nested expressions");
    cannonize.test("(5x^2)^log(cos(x),5x^2)", "cos(x)","Logarithm/Exponent inverse identity for nested expressions");

    cannonize.test("sin(-u)", "-sin(u)", "Odd function");
    cannonize.test("cos(-u)", "cos(u)", "Odd function");

    // cannonize.test("sin(u)^2 + cos(u)^2", "1", "Pythagorean identity"); // not implemented yet

    cannonize.test("derivative(x,x)", "1", "Derivative of variable with respect to itself");
    cannonize.test("derivative(sin(x),x)", "cos(x)", "Derivative of sine");
    cannonize.test("derivative(cos(x),x)", "-sin(x)", "Derivative of cosine");
    cannonize.test("derivative(exp(x),x)", "exp(x)", "Derivative of exponential function");
    cannonize.test("derivative(ln(x),x)", "1/x", "Derivative of natural log");
    cannonize.test("derivative(5,x)","0","Derivative constant rule");
    cannonize.test("derivative(213123,t)","0","Derivative constant rule");

    cannonize.test("derivative(x^2,x)","2x","Derivative power rule");
    cannonize.test("derivative(x*sin(x),x)","sin(x)+x*cos(x)","Derivative product rule");
    
    cannonize.test("derivative(x^2*y,x)","x^2*derivative(y,x)+2x*y", "Implicit differentiation and product rule");
    cannonize.test("derivative(x*y,z)","x*derivative(y,z)+y*derivative(x,z)", "Implicit differentiation and product rule");
    cannonize.test("derivative(x/y,z)","(y*derivative(x,z)-x*derivative(y,z))/y^2", "Implicit differentiation and quotient rule");

    cannonize.test("derivative(sin(cos(x^2)),x)","-2x sin(x^2)cos(cos(x^2))","nested chain rule");

    cannonize.test("derivative(5y + 2x,x)","2+5derivative(y,x)", "Derivative linearity");
    cannonize.test("derivative(2y + 32z,x)","2derivative(y,x)+32derivative(z,x)", "Derivative linearity");
    cannonize.test("derivative(derivative(x^3,x),x)", "6x", "Second derivative power rule");
    cannonize.test("derivative(x^4 + 3x^2 + 7, x)", "4x^3+6x", "Polynomial sum derivative");
    cannonize.test("derivative(1/x, x)", "-1/x^2", "Negative power derivative");
    cannonize.test("derivative(sqrt(x), x)", "1/(2*sqrt(x))", "Square root derivative rule");
    cannonize.test("derivative(x^(1/2), x)", "1/2*x^(-1/2)", "Square root derivative rule");



    std::cout<<"\n";
    cannonize.printStatus();
    std::cout<<std::endl;

    std::cout << "Thank you for testing the program!" << std::endl;

    return 0;
}



