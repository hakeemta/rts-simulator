#ifndef PFAIR_HPP
#define PFAIR_HPP

#include <iostream>
#include <cmath>

namespace PFair {
  auto Lag = [](time_t t, time_t C, time_t Ct, double U, time_t releases){
    return (t * U) - ((releases * C) - Ct);
  };

  auto Symbol = [](time_t t, time_t C, double U){
    return ((t + 1) * U) - std::floor(t * U) - 1;
  };
};

#endif
