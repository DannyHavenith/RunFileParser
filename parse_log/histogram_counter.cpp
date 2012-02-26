/*
 * histogram_counter.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */
#include <iostream>
#include "histogram_counter.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

struct histogram_counter_tool : public rtlogs::single_file_tool
{
    histogram_counter_tool()
            :single_file_tool( "histogram") {};

    virtual void run( const buffer_type &buffer,  const std::string &)
    {
        histogram_counter counter;
        scan_log( counter, buffer.begin(), buffer.end());
        counter.output( std::cout);
    }

} histogram_counter_tool_instance;

rtlogs::tool_registrar histogram_counter_tool_registrar( &histogram_counter_tool_instance);
