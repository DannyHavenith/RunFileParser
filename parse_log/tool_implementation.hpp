/*
 * tool_implementation.hpp
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#ifndef TOOL_IMPLEMENTATION_HPP_
#define TOOL_IMPLEMENTATION_HPP_
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <stdexcept>

#include "toolregistry.hpp"

using std::istreambuf_iterator;


namespace rtlogs
{

/**
 * Utility class that simplifies tool implementation.
 *
 * This class takes care of the tool name and tool argument help boilerplate.
 */
struct tool_impl : public tool
{
    tool_impl( const std::string &name, const std::string argument_help)
    :name( name), argument_help( argument_help) {}

    virtual std::string get_name() const
    {
        return name;
    }

    virtual std::string get_argument_help() const
    {
        return argument_help;
    }

protected:
    void throw_usage_error() const
    {
        throw std::runtime_error( "usage: " + name + " " + argument_help);
    }

private:
    const std::string name;
    const std::string argument_help;
};

/**
 * Base class for tools that take a single file name as a first argument.
 *
 * This class will interpret each remaining command line argument.
 */
struct single_file_tool : public tool_impl
{
    typedef std::vector<unsigned char> buffer_type;

    single_file_tool( const std::string &name)
    :tool_impl( name, "<run-file>..."){}

    virtual void run( const buffer_type &buffer, const std::string &filename) = 0;

    virtual int run( int argc, char *argv[])
    {
        using namespace std;

        if (!argc)
        {
            throw_usage_error();
        }

        while( argc)
        {


            // open the file
            ifstream file( argv[0], ios_base::binary);
            if (!file)
            {
                throw runtime_error( string("could not open file: ") + argv[1]);
            }

            // copy the file contents into a buffer
            file.unsetf( ios_base::skipws);
            istreambuf_iterator<char> begin( file), end;
            const buffer_type buffer( begin, end);

            run( buffer, argv[0]);
            --argc;
            ++argv;
        }

        return 0;
    }
};

class tool_registrar
{
public:
    tool_registrar( tool *new_tool)
    {
        tool_registry::instance().register_tool( new_tool);
    }
};

}



#endif /* TOOL_IMPLEMENTATION_HPP_ */
