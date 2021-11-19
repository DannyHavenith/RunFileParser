/*
 * toolregistry.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#include "toolregistry.hpp"

#include <map>
#include <iostream>

namespace
{
typedef std::map< std::string, rtlogs::tool *>  tool_map;
tool_map tools;
}

namespace rtlogs
{

tool_registry &tool_registry::instance()
{
    static tool_registry the_instance;
    return the_instance;
}

/**
 * Find a previously registered tool.
 */
tool *tool_registry::find_tool(const std::string & name)
{
    tool *result = nullptr;
    auto i = tools.find( name);
    if (i != tools.end())
    {
        result = i->second;
    }
    return result;
}

/**
 * print a summary of registered tools and their command line arguments to an output stream.
 */
std::ostream & tool_registry::print(std::ostream & out)
{
    for (auto i = tools.begin(); i != tools.end(); ++i)
    {
        out << i->second->get_name() << " " << i->second->get_argument_help() << '\n';
    }
    return out;
}


/**
 * register a tool.
 * The tool will be asked for its name and registered under that name.
 */
void tool_registry::register_tool(tool *new_tool)
{
    tools[new_tool->get_name()] = new_tool;
}

}
/* namespace rtlogs */
