# Build Instructions

## Prerequisites

- **Operating System:** Linux (Arch or Fedora)
- **Compiler:** GCC 14.2.1 + Intel OneAPI Toolchain
- **Essentials:** CMake, OpenSSL or GTK (optional)

## Directory Structure

- `/src/` — Core source code
- `/include/` — Headers
- `/tests/` — Unit/Integration tests
- `/docs/` — Documentation
- `/home/<user>/svakla/plugins/` — Extension modules

## Base Files

- `README.md`
- `.gitignore`
- `CMakeLists.txt`
- `.clang-format`

## Build Steps

1. **Clone the repository:**
   ```sh
   git clone https://github.com/your-repo/svaklaai.git
   cd svaklaai
   ```

2. **Install dependencies:**
   ```sh
   sudo apt-get update
   sudo apt-get install -y cmake g++ libssl-dev
   ```

3. **Build the project:**
   ```sh
   mkdir -p build
   cd build
   cmake ..
   make
   ```

4. **Run the application:**
   ```sh
   ./SvaklaAI
   ```

## Additional Notes

- Ensure that all paths are correctly set up.
- Verify that all networking components are properly configured.
- Apply default firewall security profiles.
