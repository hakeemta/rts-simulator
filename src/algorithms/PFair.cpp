#include <iostream>
#include <vector>

#include "../../includes/Task.hpp"
#include "../../includes/algorithms/PFair.hpp"

namespace PFair {
double computeLag(time_t t, const TaskParameters &params,
                  const TaskAttributes &attrs) {
  return Lag(t, params.C, attrs.Ct, params.U, attrs.releases);
}

double computeLag(const TaskParameters &params, const TaskAttributes &attrs) {
  time_t t = params.D - attrs.Dt;
  return Lag(t, params.C, attrs.Ct, params.U, 1);
};

int getSymbol(time_t t, const TaskParameters &params,
              const TaskAttributes &attrs) {
  return Symbol(t, params.C, params.U);
}

int getSymbol(const TaskParameters &params, const TaskAttributes &attrs) {
  time_t t = params.D - attrs.Dt;
  return Symbol(t, params.C, params.U);
}

std::vector<int>
PF(time_t t, const int &m,
   const std::vector<std::pair<TaskParameters, TaskAttributes>> &states) {
  std::vector<int> indices;
  std::vector<int> contendingIndices;

  for (int i = 0; i < states.size(); i++) {
    auto state = states[i];

    double lag = computeLag(t, state.first, state.second);
    double lag2 = computeLag(state.first, state.second);
    assert(std::abs(lag - lag2) < 1e-4);

    int symbol = getSymbol(t, state.first, state.second);
    int symbol2 = getSymbol(t, state.first, state.second);
    assert(symbol == symbol2);

    if ((lag > 0) && (symbol >= 0)) {
      // Urgent: behind AND +ve symbol
      indices.emplace_back(i);
    } else if ((lag < 0) && (symbol <= 0)) {
      // Tnegru: ahead AND +ve symbol, DO NOTHING
    } else {
      // Other tasks
      contendingIndices.emplace_back(i);
    }
  }

  while (indices.size() < m && contendingIndices.size() > 0) {
    contendingIndices.erase(
        std::remove_if(contendingIndices.begin(), contendingIndices.end(),
                       [&indices, &states, &t, &m](const int &i) {
                         auto state = states[i];
                         auto symbol = getSymbol(t, state.first, state.second);
                         if (symbol >= 0 && indices.size() < m) {
                           indices.emplace_back(i);
                           return true;
                         }
                         return false;
                       }),
        contendingIndices.end());

    t += 1;
  }

  return indices;
}
}; // namespace PFair
