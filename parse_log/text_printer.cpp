/*
 * text_printer.cpp
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
#include "text_printer.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

using namespace std;

struct text_printer_tool : public rtlogs::single_file_tool
{
    text_printer_tool()
            :single_file_tool( "txt") {};

    void run( const buffer_type &buffer, const std::string &) override
    {
        text_printer printer( cout);
        scan_log( printer, buffer.begin(), buffer.end());
    }
} text_printer_tool_instance;

rtlogs::tool_registrar text_printer_tool_registrar( &text_printer_tool_instance);





