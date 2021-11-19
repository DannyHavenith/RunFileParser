/*
 * histogram_counter.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */
#include "histogram_counter.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

#include <iostream>

struct histogram_counter_tool : public rtlogs::single_file_tool
{
    histogram_counter_tool()
            :single_file_tool( "histogram") {};

    void run( const buffer_type &buffer,  const std::string &) override
    {
        histogram_counter counter;
        scan_log( counter, buffer.begin(), buffer.end());
        counter.output( std::cout);
    }

} histogram_counter_tool_instance;

rtlogs::tool_registrar histogram_counter_tool_registrar( &histogram_counter_tool_instance);
