/*
 * toolregistry.hpp
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
    virtual std::string get_name() const = 0;
    virtual std::string get_argument_help() const = 0;
    virtual int run( int argc, char *argv[]) = 0;
    virtual ~tool(){};
};

class tool_registry
{
public:


    static tool_registry &instance();

    tool *find_tool( const std::string &name);
    std::ostream &print( std::ostream &out);
    void register_tool( tool *new_tool);

private:
    tool_registry(){};
    ~tool_registry(){};
};

} /* namespace rtlogs */
#endif /* TOOLREGISTRY_HPP_ */
