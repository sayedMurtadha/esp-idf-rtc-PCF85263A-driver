# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(MutexTask)

if (NOT IDF_VERSION_MAJOR)
  set(IDF_VERSION_MAJOR 3)
endif()

if (IDF_VERSION_MAJOR GREATER 3)
  set(PROJECT_ELF ${project_elf})
else()
  set(PROJECT_ELF ${IDF_PROJECT_EXECUTABLE})
endif()

target_compile_options(${PROJECT_ELF} PRIVATE -Wno-format)