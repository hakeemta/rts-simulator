#ifndef UTILS_HPP
#define UTILS_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
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

template <class T> void refresh(std::vector<std::unique_ptr<T>> &v) {
  /* Purges null (dispatched) entities.
   */
  v.erase(std::remove_if(v.begin(), v.end(),
                         [](auto &entity) { return entity == nullptr; }),
          v.end());
}

std::vector<std::tuple<time_t, time_t>>
loadTaskset(const std::string &filename) {
  std::ifstream filestream(filename);
  if (!filestream.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
    return {};
  }

  std::string line;
  std::getline(filestream, line);
  auto U = std::stod(line);

  std::getline(filestream, line);
  auto N = std::stoi(line);

  std::vector<std::tuple<time_t, time_t>> tasks;
  for (int i = 0; i < N; i++) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    time_t C, T;
    char sep;
    if (linestream >> C >> sep >> T) {
      tasks.emplace_back(C, T);
    }
  }

  return tasks;
}

#endif