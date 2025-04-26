#pragma once

#include <string>
#include <vector>

#ifdef USE_ONEAPI_DPL
#include <CL/sycl.hpp>
#else
#include <algorithm>
#include <execution>
#endif

class NLPEngine {
public:
  NLPEngine();
  ~NLPEngine();

  bool initialize();
  std::string process_input(const std::string &input);
  std::vector<float> get_embeddings(const std::string &text);

private:
  bool initialized_;
  std::vector<std::string> vocabulary_;

#ifdef USE_ONEAPI_DPL
  sycl::queue compute_queue_;
#endif
};
