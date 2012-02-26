/*
 * clean_file_writer.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include <iostream>
#include "clean_file_writer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

using namespace std;

struct clean_file_writer_tool : public rtlogs::single_file_tool
{
    clean_file_writer_tool()
            :single_file_tool( "clean") {};

    virtual void run( const buffer_type &buffer, const std::string &filename)
    {
        clean_file_writer writer( filename);
        scan_log( writer, buffer.begin(), buffer.end());
    }
} clean_file_writer_tool_instance;

rtlogs::tool_registrar clean_file_writer_tool_registrar( &clean_file_writer_tool_instance);



