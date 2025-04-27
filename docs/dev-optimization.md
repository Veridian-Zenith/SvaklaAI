# Development-Specific Optimization

## Prerequisites

- **Operating System:** Linux (Arch or Fedora)
- **Compiler:** GCC 14.2.1 + Intel OneAPI Toolchain
- **Essentials:** CMake, OpenSSL or GTK (optional)

## Optimization Steps

1. **Dev Builds Only:**
   - Utilize Intel OneAPI + XE GPU drivers.
   - Ensure all optimizations are enabled.

2. **Production Default:**
   - CPU-only mode, perfectly tuned for XP-era resource footprint.
   - No reduction in model depth, memory, reasoning chains, or creativity.

## Additional Notes

- Regularly test and benchmark optimizations.
- Ensure compatibility with all features and modules.
- Document all changes and optimizations for future reference.
