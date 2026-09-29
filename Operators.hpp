#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include <stdexcept>



using Operation = std::function<double(const std::vector<double>&)>;

class OperationRegistry {
private:
    std::unordered_map<std::string, Operation> procedureRegistry;
public:
    OperationRegistry();
    // Lookup function to execute an operation
    double execute(const std::string& op, const std::vector<double>& args) const;

    const std::unordered_map<std::string, Operation> getRegistry() const { return procedureRegistry; }

};

