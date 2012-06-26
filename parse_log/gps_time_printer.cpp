/*
 * gps_time_printer.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include <iostream>
#include "gps_time_printer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"


struct gps_timestamp_printer_tool : public rtlogs::single_file_tool
{
    gps_timestamp_printer_tool()
            :single_file_tool( "gpstime") {};

    virtual void run( const buffer_type &buffer, const std::string &filename)
    {
        // now scan the log bytes in the buffer.
        gps_timestamp_printer printer(std::cout, std::cerr, filename);
        scan_log( printer, buffer.begin(), buffer.end());
    }
} gps_timestamp_printer_tool_instance;

rtlogs::tool_registrar gps_timestamp_printer_tool_registrar( &gps_timestamp_printer_tool_instance);





