# Ensure that libc++ is used with clang++
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_CXX_FLAGS "-stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-stdlib=libc++")
set(CMAKE_SHARED_LINKER_FLAGS "-stdlib=libc++")
