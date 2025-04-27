# Plugin Development Guide

## Prerequisites

- **Programming Language:** C++
- **Build System:** CMake
- **Dependencies:** OpenSSL

## Directory Structure

- `/home/<user>/svakla/plugins/` — Dedicated plugins folder.

## Plugin Structure

- **Plugin Name:** `example_plugin.so`
- **Functions:**
  - `void example_function();`

## Development Steps

1. **Create Plugin Directory:**
   ```sh
   mkdir -p /home/<user>/svakla/plugins/example_plugin
   cd /home/<user>/svakla/plugins/example_plugin
   ```

2. **Create Plugin Source File:**
   ```cpp
   // example_plugin.cpp
   #include <iostream>

   extern "C" void example_function() {
       std::cout << "Example plugin function executed" << std::endl;
   }
   ```

3. **Create CMakeLists.txt:**
   ```cmake
   cmake_minimum_required(VERSION 3.14)
   project(example_plugin)

   add_library(example_plugin SHARED example_plugin.cpp)

   target_link_libraries(example_plugin PRIVATE OpenSSL::SSL OpenSSL::Crypto)
   ```

4. **Build the Plugin:**
   ```sh
   mkdir -p build
   cd build
   cmake ..
   make
   ```

5. **Load the Plugin:**
   ```cpp
   // In main.cpp or relevant file
   extern void load_plugin(const std::string &plugin_name);
   load_plugin("/home/<user>/svakla/plugins/example_plugin/example_plugin.so");
   ```

## Additional Notes

- Ensure all plugins are memory-isolated.
- Validate plugin signatures before activation.
- Regularly update and test plugins for compatibility.
