/*
 * toolregistry.cpp
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#include <map>
#include <iostream>
#include "toolregistry.hpp"

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


tool *tool_registry::find_tool(const std::string & name)
{
    tool *result = 0;
    tool_map::const_iterator i = tools.find( name);
    if (i != tools.end())
    {
        result = i->second;
    }
    return result;
}

std::ostream & tool_registry::print(std::ostream & out)
{
    for (tool_map::const_iterator i = tools.begin(); i != tools.end(); ++i)
    {
        out << i->second->get_name() << " " << i->second->get_argument_help() << '\n';
    }
    return out;
}


void tool_registry::register_tool(tool *new_tool)
{
    tools[new_tool->get_name()] = new_tool;
}

}
/* namespace rtlogs */
