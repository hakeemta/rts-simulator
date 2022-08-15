#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <memory>
#include <vector>

template <class T>
void copy(std::vector<std::unique_ptr<T>> &dest,
          const std::vector<std::unique_ptr<T>> &src) {
  dest.clear();
  dest.reserve(src.size());
  std::transform(
      src.begin(), src.end(), std::back_inserter(dest),
      [](const auto &entity) { return std::make_unique<T>(*entity); });
}

#endif