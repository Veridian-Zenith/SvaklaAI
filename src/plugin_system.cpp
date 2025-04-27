#include <iostream>
#include <string>
#include <dlfcn.h>
#include <unordered_map>
#include "../include/openssl_init.h"

std::unordered_map<std::string, void*> loaded_plugins;

void load_plugin(const std::string &plugin_name) {
    void *handle = dlopen(plugin_name.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open plugin: " << dlerror() << '\n';
        return;
    }
    loaded_plugins[plugin_name] = handle;
}

void unload_plugin(const std::string &plugin_name) {
    if (loaded_plugins.find(plugin_name) != loaded_plugins.end()) {
        dlclose(loaded_plugins[plugin_name]);
        loaded_plugins.erase(plugin_name);
    }
}

void* get_plugin_function(const std::string &plugin_name, const std::string &func_name) {
    if (loaded_plugins.find(plugin_name) == loaded_plugins.end()) {
        std::cerr << "Plugin not loaded: " << plugin_name << '\n';
        return nullptr;
    }
    void *func = dlsym(loaded_plugins[plugin_name], func_name.c_str());
    if (!func) {
        std::cerr << "Cannot load function: " << dlerror() << '\n';
        return nullptr;
    }
    return func;
}
