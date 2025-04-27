#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>

void initializePluginManager();
void loadPlugin(const std::string& pluginName);

#endif // PLUGIN_MANAGER_H
