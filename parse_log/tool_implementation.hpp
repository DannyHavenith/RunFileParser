/*
 * tool_implementation.hpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#ifndef TOOL_IMPLEMENTATION_HPP_
#define TOOL_IMPLEMENTATION_HPP_
#include <iostream>
#include <fstream>
#include <iterator>
#include <utility>
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "toolregistry.hpp"

namespace rtlogs
{

/**
 * Utility class that simplifies tool implementation.
 *
 * This class takes care of the tool name and tool argument help boilerplate.
 */
struct tool_impl : public tool
{
    tool_impl( std::string name, const std::string argument_help)
    :name(std::move( name)), argument_help( argument_help) {}

    [[nodiscard]] std::string get_name() const override
    {
        return name;
    }

    [[nodiscard]] std::string get_argument_help() const override
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

    int run( int argc, char *argv[]) override
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
                throw runtime_error( string("could not open file: ") + argv[0]);
            }

            // copy the file contents into a buffer
            file.unsetf( ios_base::skipws);
            std::istreambuf_iterator<char> begin( file), end;
            const buffer_type buffer( begin, end);

            run( buffer, argv[0]);
            --argc;
            ++argv;
        }

        return 0;
    }
};

/**
 * General implementation of a tool that takes an input- and an output parameter.
 *
 * Tools of this type can take a directory or a file as an input argument. If the input is a directory
 * then all files in that directory will be taken as inputs individually.
 *
 * The output argument can be omitted, can be a file (if the input argument is a file as well), or can be a directory.
 *
 * If the output argument is omitted, then the tool will create a reasonable name for the output argument. If the input argument was a
 * directory, then the created output will also be a directory.
 *
 * If more than two arguments are provided (say, n arguments), then the first n-1 will be considered input arguments. If the last argument is the name of an existing file,
 * then that argument will also be taken as an input argument and the rules will be followed as if the output argument was omitted. In all other cases, the last argument will
 * be interpreted as a directory that receives the output files.
 *
 * The concrete tool needs to implement one single function: run_on_file(const path & from, const path & to) that takes a single file as input and a single filename
 * for its output. This base class will take care of the logic around directories, multiple arguments, etc.
 */
struct input_output_tool : public tool_impl
{
    input_output_tool( const std::string &name, std::string prefix)
            : tool_impl( name, "<source>... [destination]"), prefix(std::move( prefix)) {}

    int run(int argc, char *argv[]) override
    {
        std::vector<std::string> arguments( argv, argv + argc);
        harvest_options( arguments);
        handle_options();

        if (arguments.empty() ) return -1;
        if( arguments.size() == 1)
        {
            // one source, no target specified
            invent_target_name_and_run( arguments[0]);
        }
        else if (arguments.size() == 2)
        {
            // one source, one target specified
            run_with_source_and_target_name( arguments[0], arguments[1]);
        }
        else
        {
            const path destination_dir( arguments[argc-1]);
            // multiple sources, maybe one target specified.

            if (!exists( destination_dir) || is_directory(destination_dir))
            {
                // multiple sources, one target specified
                for (int arg = 0; arg < argc - 1; ++ arg)
                {
                    const path from( arguments[arg]);
                    run_with_source_and_target_name( from, destination_dir/ from.filename());
                }
            }
            else
            {
                // multiple sources, no target specified
                for (int arg = 0; arg < argc; ++ arg)
                {
                    const path from( arguments[arg]);
                    invent_target_name_and_run( from);
                }

            }
        }


        return 0;
    }

protected:
    template<typename T, typename U>
    bool get_option( const std::string &option, T& value, U default_value) const
    {
        auto i = options.find( option);
        if (i != options.end())
        {

            try
            {
                value = boost::lexical_cast< T>(i->second);
            }
            catch( boost::bad_lexical_cast &e)
            {
                throw std::runtime_error( "could not convert value '" + i->second + "' to the right type for option '" + option + "'.");
            }
            return true;
        }
        else
        {
            value = default_value;
            return false;
        }

    }
    using OptionMap = std::map<std::string, std::string>;
    using path = boost::filesystem::path;
    virtual void run_on_file(const path & from, const path & to) =0;

    /**
     * handle all command line options (switches) before processing files.
     */
    virtual void handle_options( )
    {

    }

    virtual path invent_target_name( const path &source)
    {
        return source.parent_path() / ( prefix + source.filename().string());
    }


private:
    using StringVector = std::vector<std::string>;

    /**
     * Very simple option handling: anything that starts with a minus sign will be considered a one-argument option
     * and that argument and the following will be stored in the option map.
     */
    void harvest_options( StringVector &arguments)
    {
        auto argument = arguments.begin();
        while (argument != arguments.end())
        {
            if ((*argument)[0] == '-')
            {
                const std::string key = argument->substr(1);
                argument = arguments.erase( argument);
                if (argument != arguments.end())
                {
                    options[key] = *argument;
                    argument = arguments.erase( argument);
                }
            }
            else
            {
                ++argument;
            }
        }
    }

    void invent_target_name_and_run(const path & source)
    {
        run_with_source_and_target_name( source, invent_target_name( source));
    }

    void run_with_source_and_target_name( const path &source, const path &target)
    {
        if(is_directory(source))
        {
            run_on_directories(source, target);
        }
        else
        {
            if (is_directory( target))
            {
                run_on_file( source, target/source.filename());
            }
            else
            {
                run_on_file( source, target);
            }
        }
    }

    void run_on_directories(const path & from, const path & to)
    {
        using boost::filesystem::directory_iterator;
        bool directory_created = false;
        using path_vector = std::vector<path>;
        path_vector subdirs;
        for(directory_iterator i(from);i != directory_iterator();++i)
        {
            if(is_directory(*i))
            {
                subdirs.push_back(i->path().filename());
            }
            else
            {
                if(i->path().extension() == ".run")
                {
                    if(!directory_created)
                    {
                        create_directories( to);
                        directory_created = true;
                    }
                    run_on_file( i->path(), to / i->path().filename());
                }
            }
        }

        for (const auto & subdir : subdirs)
        {
            run_on_directories( from / subdir, to / subdir.filename());
        }

    }

private:
    OptionMap options;
    const std::string prefix;

};
}



#endif /* TOOL_IMPLEMENTATION_HPP_ */
