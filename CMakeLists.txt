
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)


cmake_minimum_required(VERSION 2.6)

project(parse_logs)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
FIND_PACKAGE( Boost COMPONENTS system filesystem)

#include_directories( ${parse_logs_SOURCE_DIR})

add_subdirectory (parse_log)



