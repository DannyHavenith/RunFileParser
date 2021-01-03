/*
 * toolregistry.hpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#ifndef TOOLREGISTRY_HPP_
#define TOOLREGISTRY_HPP_
#include <string>

namespace rtlogs
{


struct tool
{
    [[nodiscard]] virtual std::string get_name() const = 0;
    [[nodiscard]] virtual std::string get_argument_help() const = 0;
    virtual int run( int argc, char *argv[]) = 0;
    virtual ~tool()= default;;
};

class tool_registry
{
public:


    static tool_registry &instance();

    tool *find_tool( const std::string &name);
    std::ostream &print( std::ostream &out);
    void register_tool( tool *new_tool);

private:
    tool_registry() = default;
    ~tool_registry() = default;
};

} /* namespace rtlogs */
#endif /* TOOLREGISTRY_HPP_ */
