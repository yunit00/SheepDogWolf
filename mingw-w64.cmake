set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Use Clang for cross-compilation as it is more permissive with MSVC-style extensions
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

# Target flags for Clang to target Windows GNU
set(CMAKE_C_FLAGS "-target x86_64-pc-windows-gnu" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-target x86_64-pc-windows-gnu" CACHE STRING "" FORCE)

# Target environment
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Ensure we don't use static linking for the standard library as requested
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "" FORCE)
