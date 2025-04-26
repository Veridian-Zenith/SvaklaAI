#include "plugin_manager.h"
#include <dlfcn.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/evp.h>
#include <vector>

PluginManager::PluginManager(const std::string &pluginDir)
    : pluginDirectory(pluginDir) {
  if (!std::filesystem::exists(pluginDir)) {
    std::filesystem::create_directories(pluginDir);
  }
}

PluginManager::~PluginManager() { unloadAllPlugins(); }

bool PluginManager::loadPlugin(const std::string &pluginPath) {
  if (!verifyPluginSignature(pluginPath)) {
    std::cerr << "Plugin signature verification failed: " << pluginPath
              << std::endl;
    return false;
  }

  void *handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
  if (!handle) {
    std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
    return false;
  }

  // Reset errors
  dlerror();

  // Get plugin creation function
  using CreatePluginFunc = IPlugin *(*)();
  CreatePluginFunc createPlugin =
      (CreatePluginFunc)dlsym(handle, "createPlugin");

  const char *dlsym_error = dlerror();
  if (dlsym_error) {
    std::cerr << "Failed to load plugin symbols: " << dlsym_error << std::endl;
    dlclose(handle);
    return false;
  }

  // Create plugin instance
  IPlugin *instance = createPlugin();
  if (!instance) {
    std::cerr << "Failed to create plugin instance" << std::endl;
    dlclose(handle);
    return false;
  }

  // Initialize plugin
  if (!instance->initialize()) {
    std::cerr << "Plugin initialization failed: " << pluginPath << std::endl;
    delete instance;
    dlclose(handle);
    return false;
  }

  // Store plugin info
  PluginInfo info{.handle = handle, .instance = instance, .path = pluginPath};

  loadedPlugins[instance->getName()] = info;
  return true;
}

void PluginManager::unloadPlugin(const std::string &pluginName) {
  auto it = loadedPlugins.find(pluginName);
  if (it != loadedPlugins.end()) {
    it->second.instance->shutdown();
    delete it->second.instance;
    dlclose(it->second.handle);
    loadedPlugins.erase(it);
  }
}

void PluginManager::unloadAllPlugins() {
  for (const auto &[name, plugin] : loadedPlugins) {
    plugin.instance->shutdown();
    delete plugin.instance;
    dlclose(plugin.handle);
  }
  loadedPlugins.clear();
}

IPlugin *PluginManager::getPlugin(const std::string &name) {
  auto it = loadedPlugins.find(name);
  return it != loadedPlugins.end() ? it->second.instance : nullptr;
}

std::vector<std::string> PluginManager::getLoadedPlugins() const {
  std::vector<std::string> plugins;
  for (const auto &[name, _] : loadedPlugins) {
    plugins.push_back(name);
  }
  return plugins;
}

bool PluginManager::verifyPluginSignature(const std::string &pluginPath) {
  std::string sigPath = pluginPath + ".sig";
  if (!std::filesystem::exists(sigPath)) {
    return false;
  }

  // Read signature file
  std::ifstream sigFile(sigPath);
  std::string signature;
  std::getline(sigFile, signature);

  // Calculate plugin file hash using modern EVP interface
  std::ifstream pluginFile(pluginPath, std::ios::binary);
  std::vector<unsigned char> buffer(8192);

  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
  const EVP_MD *md = EVP_sha256();
  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len;

  EVP_DigestInit_ex(mdctx, md, nullptr);

  while (pluginFile.read((char *)buffer.data(), buffer.size())) {
    EVP_DigestUpdate(mdctx, buffer.data(), pluginFile.gcount());
  }

  EVP_DigestFinal_ex(mdctx, hash, &hash_len);
  EVP_MD_CTX_free(mdctx);

  // TODO: Implement actual signature verification with public key
  // For now, just check if signature file exists
  return true;
}
