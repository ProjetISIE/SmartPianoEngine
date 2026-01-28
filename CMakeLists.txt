cmake_minimum_required(VERSION 3.20)
project(
  engine # Smart Piano Engine
  VERSION 0.0.0
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTMIDI REQUIRED rtmidi)
pkg_check_modules(ALSA REQUIRED alsa)

add_compile_options(
  -Wall -Wextra -Wpedantic -Wshadow
  # -Wconversion -Werror
)

add_subdirectory(src)
enable_testing()
add_subdirectory(test)
