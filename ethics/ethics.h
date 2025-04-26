#pragma once

#include <string>
#include <vector>

class EthicsSystem {
public:
  EthicsSystem();
  ~EthicsSystem();

  bool initialize();

  // Evaluate if an action is ethically permissible
  bool evaluateAction(const std::string &action, std::string &reason);

  // Get current ethical guidelines
  std::vector<std::string> getEthicalGuidelines() const;

private:
  bool initialized;
};
