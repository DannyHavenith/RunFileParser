
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)


cmake_minimum_required(VERSION 3.2)
project(parse_logs)
include( cmake/conan.cmake)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)

conan_cmake_run(
    REQUIRES
        boost/1.73.0
    OPTIONS
        BASIC_SETUP
        CMAKE_TARGETS # individual targets to link to
    BUILD
        missing)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(
    project_options 
    INTERFACE 
        cxx_std_17)

add_subdirectory (parse_log)