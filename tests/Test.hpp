#pragma once
#include <memory>
#include <iostream>

struct TestResult
{
    bool success;
    std::string resultString;
    std::string expectedString;
};

class Test
{
    size_t totalTests;
    size_t successfulTests;
    bool showDescription;

public:
    Test():totalTests(0),successfulTests(0){};

    virtual TestResult performTest(const std::string& input, const std::string& expected) const = 0;

    void test(const std::string& input, const std::string& expected, const std::string& description)
    {
        totalTests++;
        TestResult res = performTest(input,expected);

            std::cout << "[" << (res.success? "P":"FAIL")  << "] TEST: [ " << input << " -> " << expected << " ] Result: [" << res.resultString << "]";
            
            if(!res.success)
                std::cout << " Parsed expected string after processing: " << res.expectedString;

            if(showDescription)
                std::cout << ", Description: " << description << std::endl;
            else
                std::cout << std::endl;
            
        if(res.success)
            successfulTests++;

    }

    virtual std::string getName() const = 0;

    inline void hideDescriptions(){showDescription = false;}
    inline void showDescriptions(){showDescription = true;}
    inline bool toggleDescriptions(){showDescription = !showDescription; return showDescription;}



    void printStatus()
    {
        std::cout << "'" << getName() << "' success: " << successfulTests << " / " << totalTests << " (" << 100 * successfulTests/(totalTests<=0? 1:totalTests) << "%)" << std::endl;
    }
};