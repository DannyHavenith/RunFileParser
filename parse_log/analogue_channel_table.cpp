/*
 * analogue_channel_table.cpp
 *
 *  Created on: Feb 26, 2012
 *      Author: danny
 */

#include <iostream>
#include "analogue_channel_table.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

struct analogue_channel_table_tool : public rtlogs::single_file_tool
{
    analogue_channel_table_tool()
            :single_file_tool( "values") {};

    virtual void run( const buffer_type &buffer,  const std::string &)
    {
        analogue_channel_table table(std::cout) ;
        scan_log( table, buffer.begin(), buffer.end());
        table.set_scanning(false);
        scan_log( table, buffer.begin(), buffer.end());
    }

} analogue_channel_table_tool_instance;

rtlogs::tool_registrar analogue_channel_table_tool_registrar( &analogue_channel_table_tool_instance);



