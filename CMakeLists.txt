# Version currently available in Oscar CCV as of July 2024
cmake_minimum_required(VERSION 3.20.2)

# Project name and a few useful settings. Other commands can pick up the results
project(
  jedisim
  VERSION 7.0
  DESCRIPTION "a tool to generate galaxy cluster simulations from real galaxy observational images"
  LANGUAGES C
)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Compile commands for clangd
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

endif()

# The source code is here
add_subdirectory(src)

# Update submodules as needed
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

# Fail if submodules aren't present
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/Unity/CMakeLists.txt" OR 
   NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/tomlc99/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Finally add submodules
add_subdirectory(extern/Unity)
add_subdirectory(extern/tomlc99)

include_directories(${CMAKE_SOURCE_DIR}/extern/tomlc99/include)
include_directories(${CMAKE_SOURCE_DIR}/extern/Unity/src)
link_directories(${CMAKE_BINARY_DIR}/extern/Unity)

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
