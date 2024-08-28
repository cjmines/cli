#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class CSPSolver {
public:
    using Variable = std::string;
    using Domain = std::vector<int>;
    using Assignment = std::unordered_map<Variable, int>;
    using Constraint = std::function<bool(const Assignment&)>;

    CSPSolver(const std::vector<Variable>& variables, const std::unordered_map<Variable, Domain>& domains)
        : variables(variables), domains(domains) {}
    bool solve(Assignment& assignment);
    void add_constraint(const Constraint& constraint);

private:
    std::vector<Variable> variables;
    std::unordered_map<Variable, Domain> domains;
    std::vector<Constraint> constraints;

    Variable select_unassigned_variable(const Assignment& assignment) const ;
    bool is_consistent(const Assignment& assignment) const;
};

// Example usage
int main() {
    // Variables: A, B, C
    std::vector<std::string> variables = {"A", "B", "C"};

    // Domains: A, B, C can take values from {1, 2, 3}
    std::unordered_map<std::string, std::vector<int>> domains = {
        {"A", {1, 2, 3}},
        {"B", {1, 2, 3}},
        {"C", {1, 2, 3}}
    };

    CSPSolver solver(variables, domains);

    // Add constraint: A != B
    solver.add_constraint([](const CSPSolver::Assignment& assignment) {
        return assignment.at("A") != assignment.at("B");
    });

    // Add constraint: B != C
    solver.add_constraint([](const CSPSolver::Assignment& assignment) {
        return assignment.at("B") != assignment.at("C");
    });

    // Add constraint: A != C
    solver.add_constraint([](const CSPSolver::Assignment& assignment) {
        return assignment.at("A") != assignment.at("C");
    });

    // Solve the CSP
    CSPSolver::Assignment solution;
    if (solver.solve(solution)) {
        for (const auto& [var, value] : solution) {
            std::cout << var << " = " << value << "\n";
        }
    } else {
        std::cout << "No solution found.\n";
    }

    return 0;
}
