/*
 * timestamp_printer.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#include <iostream>
#include "timestamp_printer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"


struct timestamp_printer_tool : public rtlogs::single_file_tool
{
    timestamp_printer_tool()
            :single_file_tool( "timestamps") {};

    virtual void run( const buffer_type &buffer, const std::string &)
    {
        // now scan the log bytes in the buffer.
        timestamp_printer printer(std::cout);
        scan_log( printer, buffer.begin(), buffer.end());
        // cleanup
        printer.flush();
    }
} timestamp_printer_tool_instance;

rtlogs::tool_registrar timestamp_printer_tool_registrar( &timestamp_printer_tool_instance);


