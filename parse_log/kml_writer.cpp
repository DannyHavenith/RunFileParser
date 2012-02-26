/*
 * kml_writer.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include <iostream>
#include "kml_writer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"


struct kml_writer_tool : public rtlogs::single_file_tool
{
    kml_writer_tool()
            :single_file_tool( "kml") {};

    virtual void run( const buffer_type &buffer, const std::string &)
    {
        // now scan the log bytes in the buffer.
        kml_writer printer(std::cout);
        scan_log( printer, buffer.begin(), buffer.end());
    }
} kml_writer_tool_instance;

rtlogs::tool_registrar kml_writer_tool_registrar( &kml_writer_tool_instance);


