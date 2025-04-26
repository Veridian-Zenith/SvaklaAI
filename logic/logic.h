#pragma once

#include <memory>
#include <string>
#include <vector>

class LogicEngine {
public:
  LogicEngine();
  ~LogicEngine();

  bool initialize();
  std::string evaluateExpression(const std::string &expression);
  bool validateLogic(const std::string &statement, std::string &error);

private:
  bool initialized = false;
};
