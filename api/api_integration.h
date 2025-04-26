#pragma once

#include <string>
#include <vector>

class APIIntegration {
public:
  APIIntegration();
  ~APIIntegration();

  bool initialize();
  std::string executeQuery(const std::string &query);

private:
  bool initialized = false;
};
