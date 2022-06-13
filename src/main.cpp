#include <cmath>
#include <iostream>
#include <vector>

#include "../includes/TaskSystem.hpp"
#include "../includes/algorithms/PFair.hpp"

double computeLag(time_t t, const TaskParameters &params, const TaskAttributes &attrs) {
  return PFair::Lag(t, params.C, attrs.Ct, params.U, attrs.releases);
}

double computeLag(const TaskParameters &params, const TaskAttributes &attrs) {
  time_t t = params.D - attrs.Dt + 1;
  return PFair::Lag(t, params.C, attrs.Ct, params.U, 1);
};

double getSymbol(time_t t, const TaskParameters &params, const TaskAttributes &attrs) {
  return PFair::Symbol(t, params.C, params.U);
}

double getSymbol(const TaskParameters &params, const TaskAttributes &attrs) {
  time_t t = params.D - attrs.Dt + 1;
  return PFair::Symbol(t, params.C, params.U);
}

std::vector<int> pFair(time_t t, const std::vector<std::pair<TaskParameters, TaskAttributes>> &states) {
  std::vector<int> indices;
  for (int i = 0; i < states.size(); i++) {
    auto state = states[0];

    double lag = computeLag(t, state.first, state.second);
    double lag2 = computeLag(state.first, state.second);
    assert(lag == lag2);

    int symbol = getSymbol(t, state.first, state.second);
    int symbol2 = getSymbol(state.first, state.second);
    assert(symbol == symbol2);

    if ((lag > 0) && (symbol >= 0)) {
      // Urgent: behind AND +ve symbol
      indices.emplace_back(i);
    } else if ((lag < 0) && (symbol <= 0)) {
      // Tnegru: ahead AND +ve symbol, DO NOTHING
    }
  }

  return indices;
}

int main() {
  TaskSystem system(1);
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{1, 2})));
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{3, 6})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{1, 3})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{2, 9})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{2, 9})));

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);

  std::vector<int> indices{0};
  auto states = system(indices);
  time_t t = 0;
  for (int i = 0; i < 10; i++) {
    t = system.T() + 1;
    indices = pFair(t, states);
    states = system(indices);
  }

  return 0;
}
