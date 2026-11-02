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
    std::cout << "AUTOMATIC TESTING SUITE " << std::endl;
    // cannonize.hideDescriptions();
    cannonize.showDescriptions(); // uncomment to enable descriptions

//-----Addition
    // Basic identities
    cannonize.test("x+0", "x", "Right additive identity");
    cannonize.test("0+x", "x", "Left additive identity");
    cannonize.test("x+(-x)", "0", "Additive inverse");
    cannonize.test("(-x)+x", "0", "Additive inverse");

    // Constant folding
    cannonize.test("2+3", "5", "Constant addition");
    cannonize.test("2+3+4", "9", "N-ary constant addition");
    cannonize.test("x+2+3", "x+5", "Constant collection");
    cannonize.test("2x+3x", "5x", "Like term collection");
    cannonize.test("2x+3x+5x", "10x", "Multiple like terms");
    cannonize.test("x+x", "2x", "Variable addition");
    cannonize.test("x+x+x", "3x", "N-ary variable addition");

    // Commutativity and associativity
    cannonize.test("x+y", "x+y", "Variable addition");
    cannonize.test("y+x", "x+y", "Addition commutativity");
    cannonize.test("(x+y)+z", "x+y+z", "Addition associativity");
    cannonize.test("x+(y+z)", "x+y+z", "Addition flattening");

    // Multiple variables
    cannonize.test("2x+3y+4x", "6x+3y", "Multiple variable collection");
    cannonize.test("a+b+a+b", "2a+2b", "Multiple like term collection");
    cannonize.test("3x+2y+5x+7y", "8x+9y", "Multiple variable collection");

    // Coefficient edge cases
    cannonize.test("0x+5y", "5y", "Zero coefficient elimination");
    cannonize.test("-x+x+y", "y", "Inverse cancellation");
    cannonize.test("(-2x)+5x", "3x", "Negative coefficient addition");
    cannonize.test("2(x)+3(x)", "5x", "Parenthesized like terms");

//---------- Subtraction
    // Basic identities
    cannonize.test("x-0", "x", "Right subtraction identity");
    cannonize.test("0-x", "-x", "Subtraction from zero");
    cannonize.test("x-x", "0", "Self subtraction");
    cannonize.test("x-(-x)", "2x", "Subtracting negative variable");
    cannonize.test("(-x)-x", "-2x", "Negative variable subtraction");

    // Constant folding
    cannonize.test("5-3", "2", "Constant subtraction");
    cannonize.test("3-5", "-2", "Negative constant subtraction");
    cannonize.test("x+5-3", "x+2", "Constant collection through subtraction");
    cannonize.test("10-x-5", "5-x", "Multiple subtraction");

    // Like terms
    cannonize.test("5x-2x", "3x", "Like term subtraction");
    cannonize.test("2x-5x", "-3x", "Negative like term subtraction");
    cannonize.test("x-x+x", "x", "Partial cancellation");
    cannonize.test("3x-2x-x", "0", "Complete cancellation");
    cannonize.test("5x-2y-3x", "2x-2y", "Multiple variable subtraction");

    // Subtraction associativity and grouping
    cannonize.test("(x-y)-z", "x-y-z", "Left associative subtraction");
    cannonize.test("x-(y-z)", "x-y+z", "Right grouped subtraction");
    cannonize.test("x-(y+z)", "x-y-z", "Subtraction distribution");
    cannonize.test("(x+y)-z", "x+y-z", "Subtraction of sum");
    cannonize.test("(x-y)-(z-w)", "x-y-z+w", "Nested subtraction");

    // Mixed expressions
    cannonize.test("2x-(x+3)", "x-3", "Subtraction of polynomial");
    cannonize.test("(x+2)-(x-2)", "4", "Polynomial cancellation");
    cannonize.test("3x+2-(x+5)", "2x-3", "Mixed polynomial subtraction");
//----Negation
    // Basic negation
    cannonize.test("-0", "0", "Negation of zero");
    cannonize.test("-1", "-1", "Negation of one");
    cannonize.test("--x", "x", "Double negation");
    cannonize.test("---x", "-x", "Triple negation");
    cannonize.test("----x", "x", "Quadruple negation");
    cannonize.test("-(-(-x))", "-x", "Nested negation");

    // Negation of expressions
    cannonize.test("-(x+y)", "-x-y", "Negation distribution over addition");
    cannonize.test("-(x-y)", "-x+y", "Negation distribution over subtraction");
    cannonize.test("-(-x+y)", "x-y", "Negation of mixed expression");
    cannonize.test("-(x+y+z)", "-x-y-z", "N-ary negation distribution");

    // Negation and coefficients
    cannonize.test("-2x", "-2x", "Negative coefficient");
    cannonize.test("-(-2x)", "2x", "Double negative coefficient");
    cannonize.test("-(-3x+2y)", "3x-2y", "Negation of multiple terms");
    cannonize.test("-(2x-5y)", "-2x+5y", "Negation of polynomial");

    // Negation and multiplication
    cannonize.test("(-x)*(-y)", "x*y", "Two negative factors");
    cannonize.test("(-x)*y", "-x*y", "Single negative factor");
    cannonize.test("-(x*y)", "-x*y", "Negation of product");
    cannonize.test("-2*(-3x)", "6x", "Negative coefficient multiplication");

//--- multiplication
    // Identity and annihilation
    cannonize.test("x*0", "0", "Right zero annihilation");
    cannonize.test("0*x", "0", "Left zero annihilation");
    cannonize.test("x*1", "x", "Right multiplicative identity");
    cannonize.test("1*x", "x", "Left multiplicative identity");
    cannonize.test("x*(-1)", "-x", "Multiplication by negative one");
    cannonize.test("(-1)*x", "-x", "Negative one multiplication");

    // Constant folding
    cannonize.test("2*3", "6", "Constant multiplication");
    cannonize.test("2*3*4", "24", "N-ary constant multiplication");
    cannonize.test("2*x*3", "6x", "Constant collection");
    cannonize.test("2*3*x*y", "6x*y", "Multiple constant collection");

    // Variable multiplication
    cannonize.test("x*x", "x^2", "Self multiplication");
    cannonize.test("x*x*x", "x^3", "Repeated variable multiplication");
    cannonize.test("x*y", "x y", "Variable multiplication");
    cannonize.test("y*x", "x y", "Multiplication commutativity");
    cannonize.test("(x*y)*z", "x y z", "Multiplication associativity");
    cannonize.test("x*(y*z)", "x y z", "Multiplication flattening");

    // Exponent combination
    cannonize.test("x^2*x^3", "x^5", "Same base exponent addition");
    cannonize.test("x^a*x^b", "x^(a+b)", "Symbolic exponent addition");
    cannonize.test("x^2*x", "x^3", "Exponent one combination");
    cannonize.test("x^0*x^5", "x^5", "Degree zero combination");
    cannonize.test("x^-2*x^5", "x^3", "Negative exponent combination");

    // Coefficients and degrees
    cannonize.test("2x*3x", "6x^2", "Coefficient and degree multiplication");
    cannonize.test("2x^2*3x^3", "6x^5", "Coefficient and degree propagation");
    cannonize.test("(-2x)*3x", "-6x^2", "Negative coefficient propagation");
    cannonize.test("(-2x)*(-3x)", "6x^2", "Double negative coefficient propagation");

    // Distribution
    cannonize.test("x*(y+z)", "x y+x z", "Right distributive property");
    cannonize.test("(x+y)*z", "x z+y z", "Left distributive property");
    cannonize.test("2*(x+y)", "2x+2y", "Scalar distribution");
    cannonize.test("(x+1)*2", "2x+2", "Right scalar distribution");
    cannonize.test("(x+y)*(a+b)", "x*a+x*b+y*a+y*b", "Binomial distribution");
    cannonize.test("(x+1)*(x+2)", "x^2+3x+2", "Polynomial multiplication");
    cannonize.test("(x+y)^2", "x^2+2x*y+y^2", "Binomial square expansion");
    cannonize.test("(x+y)*(x-y)", "x^2-y^2", "Difference of squares");
//---- division
    // Basic identities
    cannonize.test("0/x", "0", "Zero numerator");
    cannonize.test("x/1", "x", "Division by one");
    cannonize.test("x/x", "1", "Self division");
    cannonize.test("0/5", "0", "Zero divided by constant");
    cannonize.test("5/1", "5", "Constant division identity");

    // Constant folding
    cannonize.test("6/2", "3", "Constant division");
    cannonize.test("10/2", "5", "Integer division");
    cannonize.test("2/4", "1/2", "Fractional constant division");

    // Negative division
    cannonize.test("(-x)/y", "-(x/y)", "Negative numerator");
    cannonize.test("x/(-y)", "-(x/y)", "Negative denominator");
    cannonize.test("(-x)/(-y)", "x/y", "Double negative division");
    cannonize.test("(-6)/2", "-3", "Negative constant numerator");
    cannonize.test("6/(-2)", "-3", "Negative constant denominator");

    // Reciprocal identities
    cannonize.test("1/(1/x)", "x", "Reciprocal of reciprocal");
    cannonize.test("x*(1/x)", "1", "Multiplication by reciprocal");
    cannonize.test("(1/x)*x", "1", "Reciprocal multiplication");
    cannonize.test("x^-1", "1/x", "Negative exponent to reciprocal");
    cannonize.test("1/x^-1", "x", "Reciprocal of negative exponent");

    // Exponent manipulation
    cannonize.test("x^5/x^2", "x^3", "Same base exponent subtraction");
    cannonize.test("x^2/x^5", "x^-3", "Negative resulting exponent");
    cannonize.test("x^a/x^b", "x^(a-b)", "Symbolic exponent subtraction");
    cannonize.test("(x^2*y)/x", "x*y", "Partial factor cancellation");
    cannonize.test("(x*y)/x", "y", "Numerator factor cancellation");
    cannonize.test("(x*y)/(x*z)", "y/z", "Multiple factor cancellation");

    // Nested division
    cannonize.test("(x/y)/z", "x/(y*z)", "Left associative division");
    cannonize.test("x/(y/z)", "x*z/y", "Right grouped division");
    cannonize.test("x/(y*y)", "x/y^2", "Repeated denominator factor");
    cannonize.test("(x/y)*(y/z)", "x/z", "Cross multiplication cancellation");

    // Coefficient handling
    cannonize.test("6x/3", "2x", "Coefficient division");
    cannonize.test("6x/2x", "3", "Coefficient and variable cancellation");
    cannonize.test("4x^3/2x", "2x^2", "Coefficient and degree division");
    cannonize.test("2x^2/4x^5", "1/(2x^3)", "Negative degree and coefficient propagation");

//--- Exponentiation
    // Basic identities
    cannonize.test("x^0", "1", "Zero exponent");
    cannonize.test("x^1", "x", "First power identity");
    cannonize.test("1^x", "1", "Base one");
    cannonize.test("0^x", "0", "Base zero");
    cannonize.test("2^0", "1", "Constant zero exponent");
    cannonize.test("2^1", "2", "Constant first power");
    cannonize.test("2^3", "8", "Constant exponentiation");

    // Negative exponents
    cannonize.test("x^-1", "1/x", "Negative first exponent");
    cannonize.test("x^-2", "1/x^2", "Negative exponent");
    cannonize.test("x^-3", "1/x^3", "Negative exponent conversion");
    cannonize.test("(x^2)^-1", "1/x^2", "Negative power of power");

    // Exponent laws
    cannonize.test("x^2*x^3", "x^5", "Multiplication of equal bases");
    cannonize.test("x^5/x^2", "x^3", "Division of equal bases");
    cannonize.test("(x^2)^3", "x^6", "Power of a power");
    cannonize.test("(x^a)^b", "x^(a*b)", "Symbolic power of power");
    cannonize.test("x^(a+b)", "x^a*x^b", "Power of sum distribution");
    cannonize.test("x^(a-b)", "x^a/x^b", "Power of difference distribution");

    // Powers of products
    cannonize.test("(x*y)^2", "x^2*y^2", "Power of product");
    cannonize.test("(x*y)^3", "x^3*y^3", "Cubic power of product");
    cannonize.test("(2x)^2", "4x^2", "Coefficient exponentiation");
    cannonize.test("(3x)^3", "27x^3", "Coefficient cubic exponentiation");
    cannonize.test("(2x*y)^2", "4x^2y^2", "Multiple coefficient and degree propagation");

    // Powers of sums
    cannonize.test("(x+1)^2", "x^2+2x+1", "Binomial square");
    cannonize.test("(x+2)^2", "x^2+4x+4", "Binomial square with coefficients");
    cannonize.test("(x+1)^3", "x^3+3x^2+3x+1", "Binomial cube");
    cannonize.test("(x-1)^2", "x^2-2x+1", "Negative binomial square");
    cannonize.test("(x-1)^3", "x^3-3x^2+3x-1", "Negative binomial cube");

    // Nested expressions
    cannonize.test("((x+1)^2)^2", "x^4+4x^3+6x^2+4x+1", "Nested polynomial powers");
    cannonize.test("(x^2+1)^2", "x^4+2x^2+1", "Power of polynomial");

//----Logarithms
    // Basic identities
    cannonize.test("ln(1)", "0", "Natural logarithm of one");
    cannonize.test("ln(e)", "1", "Natural logarithm of e");
    cannonize.test("log(1,2)", "0", "Logarithm of one");
    cannonize.test("log(2,2)", "1", "Logarithm of base");
    cannonize.test("log(3,3)", "1", "Logarithm of base");

    // Exponent and logarithm inverses
    cannonize.test("log(2^x,2)", "x", "Logarithm and exponent inverse");
    cannonize.test("log(3^x,3)", "x", "Logarithm and exponent inverse");
    cannonize.test("2^log(x,2)", "x", "Exponent and logarithm inverse");
    cannonize.test("3^log(x,3)", "x", "Exponent and logarithm inverse");
    cannonize.test("ln(exp(x))", "x", "Natural logarithm inverse");
    cannonize.test("exp(ln(x))", "x", "Natural exponential inverse");

    // Power rules
    cannonize.test("log(x^2,2)", "2*log(x,2)", "Logarithm power rule");
    cannonize.test("log(x^3,2)", "3*log(x,2)", "Logarithm cubic power rule");
    cannonize.test("log(2^x,3)", "x*log(2,3)", "Symbolic exponent extraction");
    cannonize.test("log(3^x,2)", "x*log(3,2)", "Symbolic exponent extraction");
    cannonize.test("ln(x^2)", "2*ln(x)", "Natural logarithm power rule");

    // Product and quotient rules
    cannonize.test("log(x*y,2)", "log(x,2)+log(y,2)", "Logarithm product rule");
    cannonize.test("log(x/y,2)", "log(x,2)-log(y,2)", "Logarithm quotient rule");
    cannonize.test("ln(x*y)", "ln(x)+ln(y)", "Natural logarithm product rule");
    cannonize.test("ln(x/y)", "ln(x)-ln(y)", "Natural logarithm quotient rule");

    // Nested expressions
    cannonize.test("(x+2)^log(5,x+2)", "5", "Nested logarithm and exponent inverse");
    cannonize.test("(x+2)^log(5x,x+2)", "5x", "Nested expression logarithm inverse");
    cannonize.test("log((x+1)^2, x+1)", "2", "Nested logarithm simplification");

//--- Square roots
    // Basic identities
    cannonize.test("sqrt(0)", "0", "Square root of zero");
    cannonize.test("sqrt(1)", "1", "Square root of one");
    cannonize.test("sqrt(4)", "2", "Square root of perfect square");
    cannonize.test("sqrt(9)", "3", "Square root of perfect square");
    cannonize.test("sqrt(16)", "4", "Square root of perfect square");
    cannonize.test("sqrt(25)", "5", "Square root of perfect square");

    // Fractional exponent conversion
    cannonize.test("sqrt(x)", "x^(1/2)", "Square root to fractional exponent");
    cannonize.test("sqrt(x^2)", "x", "Square root of squared variable"); // Domain assumption: x >= 0
    cannonize.test("sqrt(x^4)", "x^2", "Square root of fourth power");
    cannonize.test("sqrt(x^6)", "x^3", "Square root of sixth power"); // Domain assumption: x >= 0

    // Nested roots
    cannonize.test("sqrt(sqrt(x))", "x^(1/4)", "Nested square roots");
    cannonize.test("sqrt(x^2*y^2)", "x*y", "Square root of product of squares"); // Domain assumptions
    cannonize.test("sqrt(x)*sqrt(x)", "x", "Square root multiplication");

    // Coefficient propagation
    cannonize.test("sqrt(4x^2)", "2x", "Coefficient and degree propagation"); // Domain assumption: x >= 0
    cannonize.test("sqrt(9x^4)", "3x^2", "Square root of coefficient and degree");
    cannonize.test("sqrt(16x^6)", "4x^3", "Square root of higher degree"); // Domain assumption: x >= 0

    // Square root and exponentiation
    cannonize.test("sqrt(x)^2", "x", "Square root squared");
    cannonize.test("sqrt(x^2)^2", "x^2", "Nested square root and power");

// ----sin
    // Basic identities
    cannonize.test("sin(0)", "0", "Sine of zero");
    cannonize.test("sin(-x)", "-sin(x)", "Sine odd symmetry");
    cannonize.test("sin(-(-x))", "sin(x)", "Nested negative argument");
    cannonize.test("sin(x)", "sin(x)", "Sine preservation");

    // Constant evaluation (if implemented)
    cannonize.test("sin(0)", "0", "Constant sine evaluation");
    cannonize.test("sin(pi/2)", "1", "Sine of pi over two");
    cannonize.test("sin(pi)", "0", "Sine of pi");
    cannonize.test("sin(3*pi/2)", "-1", "Sine of three pi over two");

    // Nested functions
    cannonize.test("sin(-cos(x))", "-sin(cos(x))", "Negated nested cosine");
    cannonize.test("sin(cos(-x))", "sin(cos(x))", "Nested even function");
    cannonize.test("sin(sin(-x))", "-sin(sin(x))", "Nested odd functions");

    // Multiplication and powers
    cannonize.test("sin(x)^1", "sin(x)", "Sine first power");
    cannonize.test("sin(x)^0", "1", "Sine zero power");
    cannonize.test("2*sin(x)", "2sin(x)", "Sine coefficient");
    cannonize.test("sin(x)*sin(x)", "sin(x)^2", "Sine self multiplication");
// ---- cos
    // Basic identities
    cannonize.test("cos(0)", "1", "Cosine of zero");
    cannonize.test("cos(-x)", "cos(x)", "Cosine even symmetry");
    cannonize.test("cos(-(-x))", "cos(x)", "Nested negative argument");
    cannonize.test("cos(x)", "cos(x)", "Cosine preservation");

    // Constant evaluation (if implemented)
    cannonize.test("cos(0)", "1", "Constant cosine evaluation");
    cannonize.test("cos(pi/2)", "0", "Cosine of pi over two");
    cannonize.test("cos(pi)", "-1", "Cosine of pi");
    cannonize.test("cos(2*pi)", "1", "Cosine of two pi");

    // Nested functions
    cannonize.test("cos(-sin(x))", "cos(sin(x))", "Even symmetry of nested sine");
    cannonize.test("cos(cos(-x))", "cos(cos(x))", "Nested even functions");
    cannonize.test("cos(sin(-x))", "cos(sin(x))", "Nested odd function");

    // Multiplication and powers
    cannonize.test("cos(x)^1", "cos(x)", "Cosine first power");
    cannonize.test("cos(x)^0", "1", "Cosine zero power");
    cannonize.test("2*cos(x)", "2cos(x)", "Cosine coefficient");
    cannonize.test("cos(x)*cos(x)", "cos(x)^2", "Cosine self multiplication");
// ---- derivatives
    // Constants
    cannonize.test("derivative(0,x)", "0", "Derivative of zero");
    cannonize.test("derivative(1,x)", "0", "Derivative of one");
    cannonize.test("derivative(5,x)", "0", "Derivative of constant");
    cannonize.test("derivative(213123,t)", "0", "Derivative of constant");
    cannonize.test("derivative(pi,x)", "0", "Derivative of constant symbol");

    // Variables
    cannonize.test("derivative(x,x)", "1", "Derivative of variable");
    cannonize.test("derivative(y,x)", "derivative(y,x)", "Derivative of independent variable");
    cannonize.test("derivative(x,y)", "derivative(x,y)", "Derivative with respect to another variable");
    cannonize.test("derivative(x,x)", "1", "Matching differentiation variable");

    // Basic powers
    cannonize.test("derivative(x^2,x)", "2x", "Power rule");
    cannonize.test("derivative(x^3,x)", "3x^2", "Cubic power rule");
    cannonize.test("derivative(x^4,x)", "4x^3", "Fourth power rule");
    cannonize.test("derivative(x^10,x)", "10x^9", "Higher power rule");
    cannonize.test("derivative(x^-1,x)", "-1/x^2", "Negative power rule");
    cannonize.test("derivative(x^-2,x)", "-2/x^3", "Negative exponent rule");

    // Coefficients
    cannonize.test("derivative(2x,x)", "2", "Coefficient differentiation");
    cannonize.test("derivative(5x,x)", "5", "Scalar multiplication rule");
    cannonize.test("derivative(3x^2,x)", "6x", "Coefficient power rule");
    cannonize.test("derivative(7x^3,x)", "21x^2", "Higher coefficient power rule");
    cannonize.test("derivative(-2x^2,x)", "-4x", "Negative coefficient differentiation");

    // Addition
    cannonize.test("derivative(x+y,x)", "1+derivative(y,x)", "Sum rule");
    cannonize.test("derivative(x^2+y^2,x)", "2x+2y*derivative(y,x)", "Sum of powers");
    cannonize.test("derivative(x^2+3x+7,x)", "2x+3", "Polynomial differentiation");
    cannonize.test("derivative(x^4+3x^2+7,x)", "4x^3+6x", "Polynomial sum differentiation");
    cannonize.test("derivative(5x+2y+3z,x)", "5+2derivative(y,x)+3derivative(z,x)", "N-ary sum differentiation");

    // Subtraction
    cannonize.test("derivative(x-y,x)", "1-derivative(y,x)", "Difference rule");
    cannonize.test("derivative(x^2-y^2,x)", "2x-2y*derivative(y,x)", "Difference of powers");
    cannonize.test("derivative(5x-3,x)", "5", "Constant subtraction");
    cannonize.test("derivative(x^3-2x^2+x,x)", "3x^2-4x+1", "Polynomial difference differentiation");

    // Negation
    cannonize.test("derivative(-x,x)", "-1", "Negation differentiation");
    cannonize.test("derivative(-x^2,x)", "-2x", "Negative power differentiation");
    cannonize.test("derivative(-(x+y),x)", "-1-derivative(y,x)", "Negated sum differentiation");
    cannonize.test("derivative(-5x^3,x)", "-15x^2", "Negative coefficient power rule");

    // Basic product rule
    cannonize.test("derivative(x*y,x)", "x*derivative(y,x)+y", "Product rule");
    cannonize.test("derivative(x*y,z)", "x*derivative(y,z)+y*derivative(x,z)", "Implicit product rule");
    cannonize.test("derivative(x*x,x)", "2x", "Self product rule");
    cannonize.test("derivative(x*x^2,x)", "3x^2", "Product of powers");

    // Constant multiplication
    cannonize.test("derivative(2*x,x)", "2", "Constant product rule");
    cannonize.test("derivative(5*x^2,x)", "10x", "Constant coefficient product rule");
    cannonize.test("derivative(3*x*y,x)", "3x*derivative(y,x)+3y", "Coefficient product rule");

    // Multiple factors
    cannonize.test("derivative(x*y*z,x)", "x*y*derivative(z,x)+x*z*derivative(y,x)+y*z", "Three factor product rule");
    cannonize.test("derivative(x*y*z,t)", "x*y*derivative(z,t)+x*z*derivative(y,t)+y*z*derivative(x,t)", "Three variable implicit product rule");

    // Product of expressions
    cannonize.test("derivative((x+1)*(x+2),x)", "2x+3", "Product rule with binomials");
    cannonize.test("derivative(x*(x^2+1),x)", "3x^2+1", "Product rule with polynomial");
    cannonize.test("derivative((x^2+1)*(x^2-1),x)", "4x^3", "Product rule with two polynomials");

    // Basic quotient rule
    cannonize.test("derivative(x/y,x)", "(y-x*derivative(y,x))/y^2", "Quotient rule");
    cannonize.test("derivative(x/y,z)", "(y*derivative(x,z)-x*derivative(y,z))/y^2", "Implicit quotient rule");

    // Constant denominators
    cannonize.test("derivative(x/2,x)", "1/2", "Constant denominator");
    cannonize.test("derivative(x^2/2,x)", "x", "Polynomial divided by constant");
    cannonize.test("derivative(5x/3,x)", "5/3", "Coefficient divided by constant");

    // Reciprocal differentiation
    cannonize.test("derivative(1/x,x)", "-1/x^2", "Reciprocal rule");
    cannonize.test("derivative(1/x^2,x)", "-2/x^3", "Reciprocal power rule");
    cannonize.test("derivative(1/(x+1),x)", "-1/(x+1)^2", "Reciprocal of sum");
    cannonize.test("derivative(1/(x^2+1),x)", "-2x/(x^2+1)^2", "Reciprocal of polynomial");

    // Nested quotients
    cannonize.test("derivative(x/(x+1),x)", "1/(x+1)^2", "Quotient cancellation");
    cannonize.test("derivative((x+1)/(x-1),x)", "-2/(x-1)^2", "Quotient of linear polynomials");

    // Basic power rule
    cannonize.test("derivative(x^2,x)", "2x", "Power rule");
    cannonize.test("derivative(x^3,x)", "3x^2", "Cubic power rule");
    cannonize.test("derivative(x^5,x)", "5x^4", "Higher power rule");
    cannonize.test("derivative(x^0,x)", "0", "Zero exponent differentiation");
    cannonize.test("derivative(x^1,x)", "1", "First exponent differentiation");

    // Negative and fractional exponents
    cannonize.test("derivative(x^-1,x)", "-1/x^2", "Negative exponent differentiation");
    cannonize.test("derivative(x^-3,x)", "-3/x^4", "Negative cubic exponent differentiation");
    cannonize.test("derivative(x^(1/2),x)", "1/(2*sqrt(x))", "Fractional exponent differentiation");
    cannonize.test("derivative(x^(1/3),x)", "1/3*x^(-2/3)", "Fractional exponent differentiation");

    // Coefficients
    cannonize.test("derivative(2x^3,x)", "6x^2", "Coefficient power differentiation");
    cannonize.test("derivative(5x^5,x)", "25x^4", "Higher coefficient power differentiation");

    // Variable exponent
    cannonize.test("derivative(2^x,x)", "2^x*ln(2)", "Constant base variable exponent");
    cannonize.test("derivative(x^x,x)", "x^x*(ln(x)+1)", "Variable base and exponent");
    cannonize.test("derivative(x^y,x)", "y*x^(y-1)", "Variable base constant exponent");
    cannonize.test("derivative(a^x,x)", "a^x*ln(a)", "Symbolic constant base");

    // General power rule
    cannonize.test("derivative(x^y,z)", "y*x^(y-1)*derivative(x,z)+x^y*ln(x)*derivative(y,z)", "General power rule");
    cannonize.test("derivative(x^x,x)", "x*x^(x-1)+x^x*ln(x)", "General power rule expansion");

    // Natural logarithm
    cannonize.test("derivative(ln(x),x)", "1/x", "Natural logarithm derivative");
    cannonize.test("derivative(ln(x^2),x)", "2/x", "Natural logarithm of power");
    cannonize.test("derivative(ln(5x),x)", "1/x", "Natural logarithm of coefficient product");
    cannonize.test("derivative(ln(x+1),x)", "1/(x+1)", "Natural logarithm chain rule");
    cannonize.test("derivative(ln(x^2+1),x)", "2x/(x^2+1)", "Natural logarithm of polynomial");

    // Arbitrary logarithms
    cannonize.test("derivative(log(x,2),x)", "1/(x*ln(2))", "Logarithm derivative");
    cannonize.test("derivative(log(x,10),x)", "1/(x*ln(10))", "Common logarithm derivative");
    cannonize.test("derivative(log(x,a),x)", "1/(x*ln(a))", "Symbolic base logarithm derivative");

    // Logarithm with variable argument
    cannonize.test("derivative(log(x^2,2),x)", "2/(x*ln(2))", "Logarithm of power differentiation");
    cannonize.test("derivative(log(x+1,2),x)", "1/(x+1)*1/ln(2)", "Logarithm chain rule");

    cannonize.test("derivative(sqrt(x),x)", "1/(2*sqrt(x))", "Square root derivative");
    cannonize.test("derivative(sqrt(x^2),x)", "x/sqrt(x^2)", "Square root chain rule");
    cannonize.test("derivative(sqrt(x^3),x)", "3x^2/(2*sqrt(x^3))", "Square root of power");
    cannonize.test("derivative(sqrt(x+1),x)", "1/(2*sqrt(x+1))", "Square root of sum");
    cannonize.test("derivative(sqrt(2x),x)", "1/sqrt(2x)", "Square root coefficient chain rule");
    cannonize.test("derivative(sqrt(x^2+1),x)", "x/sqrt(x^2+1)", "Square root of polynomial");
    cannonize.test("derivative(sqrt(x^4+1),x)", "2x^3/sqrt(x^4+1)", "Higher degree square root");

    // Sine
    cannonize.test("derivative(sin(x),x)", "cos(x)", "Sine derivative");
    cannonize.test("derivative(sin(2x),x)", "2cos(2x)", "Sine chain rule");
    cannonize.test("derivative(sin(x^2),x)", "2x*cos(x^2)", "Sine of power");
    cannonize.test("derivative(sin(x^3),x)", "3x^2*cos(x^3)", "Sine of cubic");
    cannonize.test("derivative(sin(x+1),x)", "cos(x+1)", "Sine of sum");
    cannonize.test("derivative(sin(-x),x)", "-cos(x)", "Sine negative argument differentiation");

    // Cosine
    cannonize.test("derivative(cos(x),x)", "-sin(x)", "Cosine derivative");
    cannonize.test("derivative(cos(2x),x)", "-2sin(2x)", "Cosine chain rule");
    cannonize.test("derivative(cos(x^2),x)", "-2x*sin(x^2)", "Cosine of power");
    cannonize.test("derivative(cos(x^3),x)", "-3x^2*sin(x^3)", "Cosine of cubic");
    cannonize.test("derivative(cos(x+1),x)", "-sin(x+1)", "Cosine of sum");
    cannonize.test("derivative(cos(-x),x)", "-sin(x)", "Cosine negative argument differentiation");

    // Nested sine and cosine
    cannonize.test("derivative(sin(cos(x)),x)", "-sin(x)*cos(cos(x))", "Nested sine and cosine");
    cannonize.test("derivative(cos(sin(x)),x)", "-cos(x)*sin(sin(x))", "Nested cosine and sine");
    cannonize.test("derivative(sin(sin(x)),x)", "cos(x)*cos(sin(x))", "Nested sine");
    cannonize.test("derivative(cos(cos(x)),x)", "sin(x)*sin(cos(x))", "Nested cosine");

    // Polynomial inside trigonometric functions
    cannonize.test("derivative(sin(x^2),x)", "2x*cos(x^2)", "Polynomial sine chain rule");
    cannonize.test("derivative(cos(x^2),x)", "-2x*sin(x^2)", "Polynomial cosine chain rule");
    cannonize.test("derivative(sin(x^3+2x),x)", "(3x^2+2)*cos(x^3+2x)", "Complex sine chain rule");
    cannonize.test("derivative(cos(x^3+2x),x)", "-(3x^2+2)*sin(x^3+2x)", "Complex cosine chain rule");

    // Multiple nested operations
    cannonize.test("derivative(sin(cos(x^2)),x)", "-2x*sin(x^2)*cos(cos(x^2))", "Three-level chain rule");
    cannonize.test("derivative(cos(sin(x^2)),x)", "-2x*cos(x^2)*sin(sin(x^2))", "Three-level chain rule");
    cannonize.test("derivative(sin(cos(sin(x))),x)", "-cos(x)*cos(cos(sin(x)))*sin(sin(x))", "Deeply nested chain rule");

    // Logarithms and powers
    cannonize.test("derivative(ln(x^2+1),x)", "2x/(x^2+1)", "Logarithm chain rule");
    cannonize.test("derivative(sin(ln(x)),x)", "cos(ln(x))/x", "Logarithm inside sine");
    cannonize.test("derivative(cos(ln(x)),x)", "-sin(ln(x))/x", "Logarithm inside cosine");
    cannonize.test("derivative(ln(sin(x)),x)", "cos(x)/sin(x)", "Sine inside logarithm");

    // Second derivatives
    cannonize.test("derivative(derivative(x^2,x),x)", "2", "Second derivative of quadratic");
    cannonize.test("derivative(derivative(x^3,x),x)", "6x", "Second derivative of cubic");
    cannonize.test("derivative(derivative(x^4,x),x)", "12x^2", "Second derivative of quartic");
    cannonize.test("derivative(derivative(sin(x),x),x)", "-sin(x)", "Second derivative of sine");
    cannonize.test("derivative(derivative(cos(x),x),x)", "-cos(x)", "Second derivative of cosine");

    // Third derivatives
    cannonize.test("derivative(derivative(derivative(x^3,x),x),x)", "6", "Third derivative of cubic");
    cannonize.test("derivative(derivative(derivative(x^4,x),x),x)", "24x", "Third derivative of quartic");
    cannonize.test("derivative(derivative(derivative(sin(x),x),x),x)", "-cos(x)", "Third derivative of sine");

    // Higher order zero
    cannonize.test("derivative(derivative(5,x),x)", "0", "Second derivative of constant");
    cannonize.test("derivative(derivative(x,x),x)", "0", "Second derivative of linear variable");


//---- Cross operator interactions

    // Addition + multiplication
    cannonize.test("(x+1)*(x+2)+x", "x^2+4x+2", "Polynomial expansion and addition");
    cannonize.test("2*(x+1)+3*(x+2)", "5x+8", "Multiple scalar distributions");
    cannonize.test("(x+y)*(x+y)", "x^2+2x*y+y^2", "Repeated binomial expansion");

    // Multiplication + division
    cannonize.test("(2x*3y)/(6x)", "y", "Coefficient and factor cancellation");
    cannonize.test("(x^2*y^3)/(x*y)", "x*y^2", "Multiple exponent cancellation");
    cannonize.test("(2x^3)/(4x)", "x^2/2", "Coefficient and degree division");

    // Negation + exponentiation
    cannonize.test("(-x)^2", "x^2", "Even power of negative expression");
    cannonize.test("(-x)^3", "-x^3", "Odd power of negative expression");
    cannonize.test("(-2x)^2", "4x^2", "Negative coefficient squared");
    cannonize.test("(-2x)^3", "-8x^3", "Negative coefficient cubed");

    // Exponentiation + logarithms
    cannonize.test("log(x^2,2)", "2*log(x,2)", "Power and logarithm interaction");
    cannonize.test("2^log(x,2)", "x", "Exponent and logarithm cancellation");
    cannonize.test("(x+1)^log(3,x+1)", "3", "Nested power and logarithm cancellation");

    // Trigonometry + algebra
    cannonize.test("sin(-x)+cos(-x)", "-sin(x)+cos(x)", "Trigonometric parity and addition");
    cannonize.test("2sin(-x)+3cos(-x)", "-2sin(x)+3cos(x)", "Trigonometric parity with coefficients");
    cannonize.test("sin(x)*cos(x)+sin(x)*cos(x)", "2sin(x)cos(x)", "Trigonometric like term collection");

    // Derivatives + expansion
    cannonize.test("derivative((x+1)^2,x)", "2x+2", "Derivative and binomial expansion");
    cannonize.test("derivative((x+1)^3,x)", "3x^2+6x+3", "Derivative and cubic expansion");
    cannonize.test("derivative((x+1)*(x+2),x)", "2x+3", "Derivative and polynomial multiplication");
    cannonize.test("derivative(2*(x^2+3x+1),x)", "4x+6", "Derivative and scalar distribution");
    cannonize.test("derivative(sin(x^2+1),x)", "2x*cos(x^2+1)", "Derivative and nested addition");
    cannonize.test("derivative((x^2+1)^2,x)", "4x*(x^2+1)", "Derivative and general chain rule");

    // Derivative + division
    cannonize.test("derivative((x^2+1)/x,x)", "1-1/x^2", "Derivative and quotient simplification");
    cannonize.test("derivative(x/(x+1),x)", "1/(x+1)^2", "Derivative and quotient cancellation");

    // Derivative + logarithms
    cannonize.test("derivative(ln(x^2),x)", "2/x", "Derivative and logarithm simplification");
    cannonize.test("derivative(ln(x^2+1),x)", "2x/(x^2+1)", "Derivative and polynomial logarithm");

    // Derivative + trigonometry
    cannonize.test("derivative(sin(2x+1),x)", "2cos(2x+1)", "Derivative and linear chain rule");
    cannonize.test("derivative(cos(3x^2),x)", "-6x*sin(3x^2)", "Derivative and nonlinear chain rule");
    cannonize.test("derivative(sin(x)*cos(x),x)", "cos(x)^2-sin(x)^2", "Derivative of trigonometric product");

    std::cout<<"\n";
    cannonize.printStatus();
    std::cout<<std::endl;

    std::cout << "Thank you for testing the program!" << std::endl;

    return 0;
}



