/**
 *
 * parse_log.cpp : Defines the entry point for the console application.
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */


#include <stdexcept>
#include <string>
#include <iostream>
#include <exception>

#include "toolregistry.hpp"

void error( const std::string &what)
{
    throw std::runtime_error( what);
}

int main(int argc, char* argv[])
{
    using namespace std;
    using namespace rtlogs;

    try
    {
        tool *the_tool = 0;
        if (argc < 2 ||  0 == (the_tool = tool_registry::instance().find_tool( argv[1])))
        {
            cerr << "usage: parse <command> [command options...]\n";
            cerr << "where command is one of:\n";
            rtlogs::tool_registry::instance().print( cerr);
        }
        else
        {
            the_tool->run( ----argc, ++++argv);
        }
    }
    catch (exception &e)
    {
        cerr << "something went wrong: " << e.what() << '\n';
    }

	return 0;
}

