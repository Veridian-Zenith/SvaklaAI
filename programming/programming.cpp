#include "programming.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

ProgrammingEngine::ProgrammingEngine() : initialized(false) {
  tempDir = fs::temp_directory_path() / "svakla_programming";
}

ProgrammingEngine::~ProgrammingEngine() {
  try {
    if (fs::exists(tempDir)) {
      fs::remove_all(tempDir);
    }
  } catch (...) {
  }
}

bool ProgrammingEngine::initialize() {
  try {
    if (!fs::exists(tempDir)) {
      fs::create_directories(tempDir);
    }
    initialized = true;
    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to initialize programming engine: " << e.what()
              << std::endl;
    return false;
  }
}

bool ProgrammingEngine::compileCode(const std::string &code,
                                    const std::string &language,
                                    std::string &output) {
  if (!initialized) {
    output = "Programming engine not initialized";
    return false;
  }

  try {
    // Generate unique filename for this compilation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) {
      ss << dis(gen);
    }
    std::string uniqueId = ss.str();

    fs::path srcPath = tempDir / (uniqueId + getSourceExtension(language));
    fs::path outPath = tempDir / (uniqueId + getExecutableExtension(language));

    // Write code to file
    std::ofstream srcFile(srcPath);
    srcFile << code;
    srcFile.close();

    // Build compilation command
    std::string command =
        buildCompileCommand(language, srcPath.string(), outPath.string());

    // Execute compilation
    FILE *pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
      output = "Failed to execute compiler";
      return false;
    }

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      result += buffer;
    }
    int status = pclose(pipe);

    // Cleanup temp files
    fs::remove(srcPath);
    if (fs::exists(outPath)) {
      fs::remove(outPath);
    }

    output = result;
    return status == 0;
  } catch (const std::exception &e) {
    output = "Compilation error: " + std::string(e.what());
    return false;
  }
}

bool ProgrammingEngine::generateAssembly(const std::string &code,
                                         const std::string &arch,
                                         std::string &assembly) {
  if (!initialized) {
    assembly = "Programming engine not initialized";
    return false;
  }

  try {
    fs::path srcPath = tempDir / "temp.cpp";
    fs::path asmPath = tempDir / "temp.s";

    // Write code to file
    std::ofstream srcFile(srcPath);
    srcFile << code;
    srcFile.close();

    // Build assembly generation command
    std::string command = "g++ -S ";
    if (arch == "x86_64") {
      command += "-m64 ";
    } else if (arch == "x86") {
      command += "-m32 ";
    }
    command += srcPath.string() + " -o " + asmPath.string();

    // Execute command
    FILE *pipe = popen((command + " 2>&1").c_str(), "r");
    if (!pipe) {
      assembly = "Failed to generate assembly";
      return false;
    }

    pclose(pipe);

    // Read generated assembly
    if (fs::exists(asmPath)) {
      std::ifstream asmFile(asmPath);
      std::stringstream buffer;
      buffer << asmFile.rdbuf();
      assembly = buffer.str();

      // Cleanup
      fs::remove(srcPath);
      fs::remove(asmPath);
      return true;
    }

    assembly = "Failed to generate assembly file";
    return false;
  } catch (const std::exception &e) {
    assembly = "Assembly generation error: " + std::string(e.what());
    return false;
  }
}

bool ProgrammingEngine::generateMakefile(
    const std::string &projectPath, const std::vector<std::string> &sources) {
  if (!initialized)
    return false;

  try {
    fs::path makefilePath = fs::path(projectPath) / "Makefile";
    std::ofstream makefile(makefilePath);

    makefile << "CXX = g++\n";
    makefile << "CXXFLAGS = -std=c++23 -Wall -O2\n\n";

    // Generate object files list
    std::string objects;
    for (const auto &src : sources) {
      fs::path srcPath(src);
      objects += srcPath.stem().string() + ".o ";
    }

    makefile << "OBJECTS = " << objects << "\n\n";
    makefile << "all: program\n\n";
    makefile << "program: $(OBJECTS)\n";
    makefile << "\t$(CXX) $(OBJECTS) -o program\n\n";

    // Generate rules for each source file
    for (const auto &src : sources) {
      fs::path srcPath(src);
      makefile << srcPath.stem().string() << ".o: " << src << "\n";
      makefile << "\t$(CXX) $(CXXFLAGS) -c " << src << "\n\n";
    }

    makefile << "clean:\n";
    makefile << "\trm -f $(OBJECTS) program\n";

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to generate Makefile: " << e.what() << std::endl;
    return false;
  }
}

bool ProgrammingEngine::createKernelModule(const std::string &moduleName,
                                           const std::string &code) {
  if (!initialized)
    return false;

  try {
    fs::path modulePath = tempDir / moduleName;
    fs::create_directories(modulePath);

    // Write module source
    std::ofstream srcFile(modulePath / (moduleName + ".c"));
    srcFile << code;
    srcFile.close();

    // Generate Makefile for kernel module
    std::ofstream makefile(modulePath / "Makefile");
    makefile << "obj-m += " << moduleName << ".o\n\n";
    makefile << "all:\n";
    makefile << "\tmake -C /lib/modules/$(shell uname -r)/build M=$(PWD) "
                "modules\n\n";
    makefile << "clean:\n";
    makefile
        << "\tmake -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean\n";
    makefile.close();

    return true;
  } catch (const std::exception &e) {
    std::cerr << "Failed to create kernel module: " << e.what() << std::endl;
    return false;
  }
}

// Private helper methods
std::string ProgrammingEngine::getSourceExtension(const std::string &language) {
  if (language == "cpp" || language == "c++")
    return ".cpp";
  if (language == "c")
    return ".c";
  if (language == "rust")
    return ".rs";
  return ".txt";
}

std::string
ProgrammingEngine::getExecutableExtension(const std::string &language) {
#ifdef _WIN32
  return ".exe";
#else
  return "";
#endif
}

std::string ProgrammingEngine::buildCompileCommand(const std::string &language,
                                                   const std::string &input,
                                                   const std::string &output) {
  if (language == "cpp" || language == "c++") {
    return "g++ -std=c++23 " + input + " -o " + output;
  }
  if (language == "c") {
    return "gcc " + input + " -o " + output;
  }
  if (language == "rust") {
    return "rustc " + input + " -o " + output;
  }
  return "";
}

bool ProgrammingEngine::setupCompiler(const std::string &language) {
  // Check if compiler is installed
  std::string checkCmd;
  if (language == "cpp" || language == "c++" || language == "c") {
    checkCmd = "which g++ >/dev/null 2>&1";
  } else if (language == "rust") {
    checkCmd = "which rustc >/dev/null 2>&1";
  } else {
    return false;
  }

  return system(checkCmd.c_str()) == 0;
}

bool ProgrammingEngine::setupAssembler(const std::string &arch) {
  // Check if appropriate assembler is available
  if (arch == "x86_64" || arch == "x86") {
    return system("which as >/dev/null 2>&1") == 0;
  }
  return false;
}
