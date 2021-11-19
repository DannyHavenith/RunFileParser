/*
 * kml_writer.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include "kml_writer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

#include <iostream>


struct kml_writer_tool : public rtlogs::single_file_tool
{
    kml_writer_tool()
            :single_file_tool( "kml") {};

    void run( const buffer_type &buffer, const std::string &) override
    {
        // now scan the log bytes in the buffer.
        kml_writer printer(std::cout);
        scan_log( printer, buffer.begin(), buffer.end());
    }
} kml_writer_tool_instance;

rtlogs::tool_registrar kml_writer_tool_registrar( &kml_writer_tool_instance);


