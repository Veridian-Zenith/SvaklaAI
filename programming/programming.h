#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class ProgrammingEngine {
public:
  ProgrammingEngine();
  ~ProgrammingEngine();

  bool initialize();
  bool compileCode(const std::string &code, const std::string &language,
                   std::string &output);
  bool generateAssembly(const std::string &code, const std::string &arch,
                        std::string &assembly);
  bool generateMakefile(const std::string &projectPath,
                        const std::vector<std::string> &sources);
  bool createKernelModule(const std::string &moduleName,
                          const std::string &code);

private:
  bool initialized = false;
  fs::path tempDir;

  bool setupCompiler(const std::string &language);
  bool setupAssembler(const std::string &arch);

  // Add missing helper function declarations
  std::string getSourceExtension(const std::string &language);
  std::string getExecutableExtension(const std::string &language);
  std::string buildCompileCommand(const std::string &language,
                                  const std::string &input,
                                  const std::string &output);
};
