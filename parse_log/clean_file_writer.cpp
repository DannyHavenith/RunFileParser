/*
 * clean_file_writer.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include "clean_file_writer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

struct clean_file_writer_tool : public rtlogs::single_file_tool
{
    clean_file_writer_tool()
            :single_file_tool( "clean") {};

    void run( const buffer_type &buffer, const std::string &filename) override
    {
        clean_file_writer writer( filename);
        scan_log( writer, buffer.begin(), buffer.end());
    }
} clean_file_writer_tool_instance;

rtlogs::tool_registrar clean_file_writer_tool_registrar( &clean_file_writer_tool_instance);



