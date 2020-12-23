
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)


cmake_minimum_required(VERSION 3.0)

project(parse_logs)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)
find_package( Boost 1.71 REQUIRED COMPONENTS filesystem regex)

add_subdirectory (parse_log)



