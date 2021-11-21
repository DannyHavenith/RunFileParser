
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)


cmake_minimum_required(VERSION 3.2)
project(parse_logs)
include( cmake/conan.cmake)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)

# cross-compile on linux for Windows.
# to do this, invoke:
# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../Toolchain-cross-mingw32-linux.cmake -DBUILD_FOR_WINDOWS=YES -G Ninja ../..
if(${BUILD_FOR_WINDOWS})
    set( EXTRA_PROFILE PROFILE ${CMAKE_SOURCE_DIR}/linux_to_windows.conanprofile)
    #set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ ${CMAKE_CXX_STANDARD_LIBRARIES}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -Wl,--no-whole-archive")
endif(${BUILD_FOR_WINDOWS})

conan_cmake_run(
    REQUIRES
        boost/1.75.0

    OPTIONS
        boost:without_fiber=True
        boost:without_stacktrace=True
        boost:without_wave=True
    
    BASIC_SETUP
    CMAKE_TARGETS # individual targets to link to
    BUILD
        missing
    BUILD_TYPE Release
    ${EXTRA_PROFILE}
)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(
    project_options 
    INTERFACE 
        cxx_std_17)

add_subdirectory (parse_log)