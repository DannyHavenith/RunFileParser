/*
 * analogue_channel_table.cpp
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
#include "analogue_channel_table.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

struct analogue_channel_table_tool : public rtlogs::single_file_tool
{
    analogue_channel_table_tool()
            :single_file_tool( "values") {};

    void run( const buffer_type &buffer,  const std::string &) override
    {
        analogue_channel_table table(std::cout) ;
        scan_log( table, buffer.begin(), buffer.end());
        table.set_scanning(false);
        scan_log( table, buffer.begin(), buffer.end());
    }

} analogue_channel_table_tool_instance;

rtlogs::tool_registrar analogue_channel_table_tool_registrar( &analogue_channel_table_tool_instance);



