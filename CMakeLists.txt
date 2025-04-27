cmake_minimum_required(VERSION 3.14)

# Project name and version
project(SvaklaAI VERSION 1.0.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable C++17 features
set(CMAKE_CXX_EXTENSIONS OFF)

# Add the source and include directories
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

# Add the executable
add_executable(SvaklaAI ${SOURCE_DIR}/main.cpp ${SOURCE_DIR}/src/http_server.cpp ${SOURCE_DIR}/src/websocket_server.cpp ${SOURCE_DIR}/src/api_server.cpp ${SOURCE_DIR}/src/firewall_script.sh ${SOURCE_DIR}/src/auth.cpp ${SOURCE_DIR}/src/web_interface.cpp ${SOURCE_DIR}/src/ai_core.cpp ${SOURCE_DIR}/src/external_service_interface.cpp ${SOURCE_DIR}/src/plugin_system.cpp ${SOURCE_DIR}/src/local_memory_storage.cpp ${SOURCE_DIR}/src/interactive_shell.cpp ${SOURCE_DIR}/src/monitoring_safety.cpp ${SOURCE_DIR}/src/advanced_low_level.cpp ${SOURCE_DIR}/src/privacy_security.cpp ${SOURCE_DIR}/src/expansion_modules.cpp ${SOURCE_DIR}/src/installation_system.cpp ${SOURCE_DIR}/src/final_summary.cpp)

# Include directories for the project
target_include_directories(SvaklaAI PRIVATE
    ${INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/api
    ${CMAKE_SOURCE_DIR}/chat
    ${CMAKE_SOURCE_DIR}/ethics
    ${CMAKE_SOURCE_DIR}/list
    ${CMAKE_SOURCE_DIR}/logic
    ${CMAKE_SOURCE_DIR}/model
    ${CMAKE_SOURCE_DIR}/nlp
    ${CMAKE_SOURCE_DIR}/plugins
    ${CMAKE_SOURCE_DIR}/programming
    ${CMAKE_SOURCE_DIR}/scraping
    ${CMAKE_SOURCE_DIR}/web_interface
)

# Link libraries
find_package(OpenSSL REQUIRED)
target_link_libraries(SvaklaAI PRIVATE OpenSSL::SSL OpenSSL::Crypto)

# Add subdirectories for the project
# Add subdirectories for the project
add_subdirectory(api)
add_subdirectory(chat)
add_subdirectory(ethics)
add_subdirectory(list)
add_subdirectory(logic)
add_subdirectory(model)
add_subdirectory(nlp)
add_subdirectory(plugins)
add_subdirectory(programming)
add_subdirectory(scraping)
add_subdirectory(tests)
add_subdirectory(web_interface)


# Install rules
install(TARGETS SvaklaAI DESTINATION bin)
