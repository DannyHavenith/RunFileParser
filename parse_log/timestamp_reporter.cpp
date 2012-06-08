/*
 * timestamp_reporter.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include <iostream>
#include "timestamp_reporter.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

using namespace std;

struct timestamp_reporter_tool : public rtlogs::single_file_tool
{
    timestamp_reporter_tool()
            :single_file_tool( "timestamp") {};

    virtual void run( const buffer_type &buffer, const std::string &filename)
    {
        timestamp_reporter printer( cout);
        scan_log( printer, buffer.begin(), buffer.end());
        if (!printer.gps_found())
        {
            cerr << "no gps timestamps in " << filename << '\n';
        }
    }
} timestamp_reporter_tool_instance;

rtlogs::tool_registrar timestamp_reporter_tool_registrar( &timestamp_reporter_tool_instance);







