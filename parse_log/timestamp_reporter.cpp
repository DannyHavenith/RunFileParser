/*
 * timestamp_reporter.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include "timestamp_reporter.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"
#include "register_tool.hpp"

#include <iostream>

using namespace std;

struct timestamp_reporter_tool : public rtlogs::single_file_tool
{
    timestamp_reporter_tool()
            :single_file_tool( "timestamp") {};

    void run( const buffer_type &buffer, const std::string &filename) override
    {
        timestamp_reporter printer( cout);
        scan_log( printer, buffer.begin(), buffer.end());
        if (!printer.gps_found())
        {
            cerr << "no gps timestamps in " << filename << '\n';
        }
    }
};

template void register_tool<timestamp_reporter_tool>();







