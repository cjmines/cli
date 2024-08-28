#include "csp.hpp"

bool CSPSolver::solve(CSPSolver::Assignment &assignment) {
  // If all variables are assigned, return true
  if (assignment.size() == variables.size()) {
    return true;
  }

  // Select an unassigned variable
  Variable var = select_unassigned_variable(assignment);

  // Iterate over all possible domain values
  for (int value : domains.at(var)) {
    assignment[var] = value;

    // Check if the assignment satisfies all constraints
    if (is_consistent(assignment)) {
      // Recursively solve the rest of the problem
      if (solve(assignment)) {
        return true;
      }
    }

    // If the assignment doesn't lead to a solution, undo it
    assignment.erase(var);
  }

  // If no value led to a solution, return false
  return false;
}

void add_constraint(const CSPSolver::Constraint &constraint) {
  constraints.push_back(constraint);
}

CSPSolver::Variable
CSPSolver::select_unassigned_variable(const Assignment &assignment) const {
  for (const auto &var : variables) {
    if (assignment.find(var) == assignment.end()) {
      return var;
    }
  }
  return ""; // Should never reach here
}

bool CSPSolver::is_consistent(const Assignment &assignment) const {
  for (const auto &constraint : constraints) {
    if (!constraint(assignment)) {
      return false;
    }
  }
  return true;
}
