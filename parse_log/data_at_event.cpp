/*
 * event_position.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include "data_at_event.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

#include <iostream>


struct data_at_event_tool : public rtlogs::single_file_tool
{
    data_at_event_tool()
            :single_file_tool( "event") {};

    void run( const buffer_type &buffer, const std::string &filename) override
    {
        // only emit a header the very first time we run
        static bool emit_header = true;

        // scan the log bytes in the buffer.
        data_at_event printer(filename, std::cout, emit_header);
        scan_log( printer, buffer.begin(), buffer.end());

        emit_header = false;
    }

} data_at_event_tool_instance;

rtlogs::tool_registrar event_position_tool_registrar( &data_at_event_tool_instance);
