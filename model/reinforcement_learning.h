#pragma once

#include <map>
#include <string>
#include <vector>

class ReinforcementLearning {
public:
  ReinforcementLearning();
  ~ReinforcementLearning();

  bool initialize();
  void train(const std::string &state, const std::string &action, float reward);
  std::string predictBestAction(const std::string &state);
  void saveModel(const std::string &path);
  bool loadModel(const std::string &path);

private:
  bool initialized = false;
  std::map<std::string, std::map<std::string, float>> qTable;
};
