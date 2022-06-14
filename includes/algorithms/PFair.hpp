#ifndef PFAIR_HPP
#define PFAIR_HPP

#include <cmath>
#include <iostream>
#include <vector>

namespace PFair {
auto Lag = [](time_t t, time_t C, time_t Ct, double U, time_t releases) {
  return (t * U) - ((releases * C) - Ct);
};

template <typename T> int Sgn(T val) { return (T(0) < val) - (val < T(0)); }

auto Symbol = [](time_t t, time_t C, double U) {
  double value = ((t + 1) * U) - std::floor(t * U) - 1;
  return Sgn(value);
};

double computeLag(time_t t, const TaskParameters &params,
                  const TaskAttributes &attrs);

double computeLag(const TaskParameters &params, const TaskAttributes &attrs);

int getSymbol(time_t t, const TaskParameters &params,
              const TaskAttributes &attrs);

int getSymbol(const TaskParameters &params, const TaskAttributes &attrs);

std::vector<int>
PF(time_t t, const int &m,
   const std::vector<std::pair<TaskParameters, TaskAttributes>> &states);
}; // namespace PFair

#endif
