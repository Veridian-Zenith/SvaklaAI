#include "plugin_manager.h"
#include <iostream>

void initializePluginManager() {
    std::cout << "Plugin manager initialized." << std::endl;
}

void loadPlugin(const std::string& pluginName) {
    std::cout << "Loading plugin: " << pluginName << std::endl;
}
