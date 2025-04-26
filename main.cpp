#include "api/api_integration.h"
#include "chat/chat_storage.h"
#include "ethics/ethics.h"
#include "logic/logic.h"
#include "model/reinforcement_learning.h"
#include "nlp/nlp.h"
#include "plugins/plugin_manager.h"
#include "programming/programming.h"
#include "scraping/web_scraping.h"
#include "web_interface/web_interface.h"
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <signal.h>
#include <thread>

namespace fs = std::filesystem;
namespace po = boost::program_options;

// Global components
std::unique_ptr<APIIntegration> g_apiIntegration;
std::unique_ptr<ChatStorage> g_chatStorage;
std::unique_ptr<EthicsSystem> g_ethicsEngine;
std::unique_ptr<LogicEngine> g_logicEngine;
std::unique_ptr<NLPEngine> g_nlpEngine;
std::unique_ptr<PluginManager> g_pluginManager;
std::unique_ptr<ProgrammingEngine> g_programmingEngine;
std::unique_ptr<ReinforcementLearning> g_rlEngine;
std::unique_ptr<WebScraper> g_webScraping;
std::unique_ptr<WebInterface> g_webInterface;

// Global IO context
boost::asio::io_context g_ioContext;
bool g_running = true;

void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::cout << "\nShutting down SvaklaAI..." << std::endl;
    g_running = false;
    g_ioContext.stop();
  }
}

void setupDirectories() {
  const fs::path home = fs::path(getenv("HOME"));
  const fs::path base = home / "svakla";

  std::vector<fs::path> dirs = {base / "plugins", base / "config",
                                base / "data", base / "chat", base / "temp"};

  for (const auto &dir : dirs) {
    if (!fs::exists(dir)) {
      try {
        fs::create_directories(dir);
      } catch (const std::exception &e) {
        std::cerr << "Failed to create directory " << dir << ": " << e.what()
                  << std::endl;
        exit(1);
      }
    }
  }
}

void initializeComponents() {
  const fs::path home = fs::path(getenv("HOME"));
  const fs::path base = home / "svakla";

  try {
    // Initialize all components
    g_apiIntegration = std::make_unique<APIIntegration>();
    g_chatStorage = std::make_unique<ChatStorage>((base / "chat").string());
    g_ethicsEngine = std::make_unique<EthicsSystem>();
    g_logicEngine = std::make_unique<LogicEngine>();
    g_nlpEngine = std::make_unique<NLPEngine>();
    g_pluginManager =
        std::make_unique<PluginManager>((base / "plugins").string());
    g_programmingEngine = std::make_unique<ProgrammingEngine>();
    g_rlEngine = std::make_unique<ReinforcementLearning>();
    g_webScraping = std::make_unique<WebScraper>();
    g_webInterface = std::make_unique<WebInterface>(g_ioContext);

    // Initialize each component
    if (!g_apiIntegration->initialize())
      throw std::runtime_error("Failed to initialize API integration");
    if (!g_chatStorage->initialize())
      throw std::runtime_error("Failed to initialize chat storage");
    if (!g_ethicsEngine->initialize())
      throw std::runtime_error("Failed to initialize ethics engine");
    if (!g_logicEngine->initialize())
      throw std::runtime_error("Failed to initialize logic engine");
    if (!g_nlpEngine->initialize())
      throw std::runtime_error("Failed to initialize NLP engine");
    if (!g_programmingEngine->initialize())
      throw std::runtime_error("Failed to initialize programming engine");
    if (!g_rlEngine->initialize())
      throw std::runtime_error("Failed to initialize reinforcement learning");
    if (!g_webScraping->initialize())
      throw std::runtime_error("Failed to initialize web scraping");

    // Load plugins
    for (const auto &entry : fs::directory_iterator(base / "plugins")) {
      if (entry.path().extension() == ".so") {
        if (!g_pluginManager->loadPlugin(entry.path().string())) {
          std::cerr << "Warning: Failed to load plugin: " << entry.path()
                    << std::endl;
        }
      }
    }

    // Initialize web interface last
    if (!g_webInterface->initialize(992, 776, 933)) {
      throw std::runtime_error("Failed to initialize web interface");
    }

  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize components: " << e.what() << std::endl;
    exit(1);
  }
}

void cleanup() {
  // Clean up in reverse order of initialization
  g_webInterface.reset();
  g_webScraping.reset();
  g_rlEngine.reset();
  g_programmingEngine.reset();
  g_pluginManager.reset();
  g_nlpEngine.reset();
  g_logicEngine.reset();
  g_ethicsEngine.reset();
  g_chatStorage.reset();
  g_apiIntegration.reset();
}

int main(int argc, char *argv[]) {
  try {
    // Set up signal handling
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Parse command line options
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")("no-web",
                                                       "disable web interface")(
        "port", po::value<int>()->default_value(992), "web interface port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    std::cout << "Starting SvaklaAI...\n" << std::endl;

    // Setup required directories
    setupDirectories();

    // Initialize core components
    initializeComponents();

    std::cout << "SvaklaAI initialized successfully!\n" << std::endl;
    std::cout << "Web interface running on:\n";
    std::cout << "- HTTP/REST: port 992\n";
    std::cout << "- WebSocket: port 776\n";
    std::cout << "- Direct API: port 933\n" << std::endl;

    // Run the IO context
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
        work = boost::asio::make_work_guard(g_ioContext);

    // Run the io_context on multiple threads
    std::vector<std::thread> threads;
    const auto thread_count = std::thread::hardware_concurrency();
    threads.reserve(thread_count);

    for (unsigned i = 1; i < thread_count; ++i) {
      threads.emplace_back([&]() { g_ioContext.run(); });
    }

    // Run the main thread's io_context
    g_ioContext.run();

    // Wait for all threads to complete
    for (auto &thread : threads) {
      thread.join();
    }

    // Cleanup on exit
    cleanup();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
