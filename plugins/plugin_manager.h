#pragma once

#include <dlfcn.h>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Interface that all plugins must implement
class IPlugin {
public:
  virtual ~IPlugin() = default;
  virtual const char *getName() = 0;
  virtual const char *getVersion() = 0;
  virtual bool initialize() = 0;
  virtual void shutdown() = 0;
};

class PluginManager {
public:
  PluginManager(const std::string &pluginDir = "/opt/svakla/plugins/");
  ~PluginManager();

  bool loadPlugin(const std::string &pluginPath);
  void unloadPlugin(const std::string &pluginName);
  void unloadAllPlugins();

  IPlugin *getPlugin(const std::string &name);
  std::vector<std::string> getLoadedPlugins() const;
  bool verifyPluginSignature(const std::string &pluginPath);

private:
  struct PluginInfo {
    void *handle;
    IPlugin *instance;
    std::string path;
  };

  std::map<std::string, PluginInfo> loadedPlugins;
  std::string pluginDirectory;
};
