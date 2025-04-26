#include "nlp.h"
#include <CL/sycl.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sentencepiece_processor.h>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

class TokenizerImpl {
private:
  sentencepiece::SentencePieceProcessor processor;
  sycl::queue &compute_queue;

public:
  TokenizerImpl(sycl::queue &queue) : compute_queue(queue) {
    auto status = processor.Load("models/svakla_tokenizer.model");
    if (!status.ok()) {
      throw std::runtime_error("Failed to load tokenizer model");
    }
  }

  std::vector<int> encode(const std::string &text) {
    std::vector<int> ids;
    processor.Encode(text, &ids);
    return ids;
  }
};

NLPEngine::NLPEngine() : initialized_(false) {
#ifdef USE_ONEAPI_DPL
  try {
    compute_queue_ = sycl::queue(sycl::gpu_selector_v);
    std::cout
        << "NLP Engine using device: "
        << compute_queue_.get_device().get_info<sycl::info::device::name>()
        << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize GPU queue: " << e.what() << std::endl;
    compute_queue_ = sycl::queue(sycl::cpu_selector_v);
  }
#endif
}

NLPEngine::~NLPEngine() {
  // Cleanup if needed
}

bool NLPEngine::initialize() {
  try {
    // Load vocabulary and models
    std::ifstream vocab_file("models/vocabulary.txt");
    if (!vocab_file) {
      std::cerr << "Failed to load vocabulary file" << std::endl;
      return false;
    }

    std::string word;
    while (std::getline(vocab_file, word)) {
      vocabulary_.push_back(word);
    }

    initialized_ = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "NLP initialization error: " << e.what() << std::endl;
    return false;
  }
}

std::vector<std::string> NLPEngine::tokenize(const std::string &text) {
  if (!initialized_)
    return {};

  std::vector<std::string> tokens;
  std::stringstream ss(text);
  std::string token;

  while (ss >> token) {
    // Basic cleaning
    token.erase(
        std::remove_if(token.begin(), token.end(),
                       [](char c) { return !std::isalnum(c) && c != '_'; }),
        token.end());

    if (!token.empty()) {
      std::transform(token.begin(), token.end(), token.begin(), ::tolower);
      tokens.push_back(token);
    }
  }

  return tokens;
}

std::string NLPEngine::extractIntent(const std::string &text) {
  if (!initialized_)
    return "unknown";

  // Simple keyword-based intent detection
  const std::unordered_map<std::string, std::vector<std::string>>
      intentKeywords = {{"search", {"find", "search", "look", "locate"}},
                        {"create", {"create", "make", "new", "generate"}},
                        {"delete", {"delete", "remove", "destroy"}},
                        {"update", {"update", "modify", "change", "edit"}},
                        {"help", {"help", "assist", "support", "guide"}}};

  auto tokens = tokenize(text);
  std::unordered_map<std::string, int> intentScores;

  for (const auto &token : tokens) {
    for (const auto &[intent, keywords] : intentKeywords) {
      if (std::find(keywords.begin(), keywords.end(), token) !=
          keywords.end()) {
        intentScores[intent]++;
      }
    }
  }

  if (intentScores.empty())
    return "unknown";

  auto highestScore = std::max_element(
      intentScores.begin(), intentScores.end(),
      [](const auto &a, const auto &b) { return a.second < b.second; });

  return highestScore->first;
}

std::vector<std::pair<std::string, std::string>>
NLPEngine::extractEntities(const std::string &text) {
  if (!initialized_)
    return {};

  std::vector<std::pair<std::string, std::string>> entities;
  auto tokens = tokenize(text);

  // Simple pattern matching for basic entities
  for (size_t i = 0; i < tokens.size(); ++i) {
    // File paths
    if (tokens[i].find('/') != std::string::npos) {
      entities.emplace_back("path", tokens[i]);
    }
    // Numbers
    else if (std::all_of(tokens[i].begin(), tokens[i].end(), ::isdigit)) {
      entities.emplace_back("number", tokens[i]);
    }
    // URLs
    else if (tokens[i].starts_with("http") || tokens[i].starts_with("www.")) {
      entities.emplace_back("url", tokens[i]);
    }
    // Simple email detection
    else if (tokens[i].find('@') != std::string::npos) {
      entities.emplace_back("email", tokens[i]);
    }
  }

  return entities;
}

float NLPEngine::calculateSimilarity(const std::string &text1,
                                     const std::string &text2) {
  if (!initialized_)
    return 0.0f;

  auto tokens1 = tokenize(text1);
  auto tokens2 = tokenize(text2);

  // Using Jaccard similarity
  std::vector<std::string> intersection;
  std::vector<std::string> union_;

  std::sort(tokens1.begin(), tokens1.end());
  std::sort(tokens2.begin(), tokens2.end());

  std::set_intersection(tokens1.begin(), tokens1.end(), tokens2.begin(),
                        tokens2.end(), std::back_inserter(intersection));

  std::set_union(tokens1.begin(), tokens1.end(), tokens2.begin(), tokens2.end(),
                 std::back_inserter(union_));

  if (union_.empty())
    return 0.0f;
  return static_cast<float>(intersection.size()) /
         static_cast<float>(union_.size());
}

std::string NLPEngine::process_input(const std::string &input) {
  if (!initialized_) {
    throw std::runtime_error("NLP Engine not initialized");
  }

#ifdef USE_ONEAPI_DPL
  // Use GPU acceleration if available
  try {
    sycl::buffer<char> input_buf(input.data(), input.size());
    compute_queue_
        .submit([&](sycl::handler &h) {
          auto acc = input_buf.get_access<sycl::access::mode::read_write>(h);
          // Process input using GPU
        })
        .wait();
  } catch (const std::exception &e) {
    std::cerr << "GPU processing error: " << e.what() << std::endl;
    // Fall back to CPU processing
  }
#else
  // Use standard parallel algorithms
  std::string processed = input;
  std::transform(std::execution::par_unseq, processed.begin(), processed.end(),
                 processed.begin(), ::tolower);
#endif

  return input;
}

std::vector<float> NLPEngine::get_embeddings(const std::string &text) {
  if (!initialized_) {
    throw std::runtime_error("NLP Engine not initialized");
  }

  // Placeholder for embeddings generation
  return std::vector<float>(768, 0.0f); // 768-dimensional embeddings
}
