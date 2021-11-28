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

#include "toolregistry.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>

#include <stdexcept>
#include <vector>
#include <string>
#include <map>

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
    void throw_usage_error() const;

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
    int run( int argc, char *argv[]) override;
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

        int run( int argc, char *argv[]) override;

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
     virtual void handle_options();
     virtual path invent_target_name( const path &source);


private:
    using StringVector = std::vector<std::string>;

    /**
    * Very simple option handling: anything that starts with a minus sign will be considered a one-argument option
    * and that argument and the following will be stored in the option map.
    */
    void harvest_options( StringVector &arguments);

    void invent_target_name_and_run( const path &source);

    void run_with_source_and_target_name(
            const path &source,
            const path &target);

    void run_on_directories( const path &from, const path &to);

private:
    OptionMap options;
    const std::string prefix;

};

}



#endif /* TOOL_IMPLEMENTATION_HPP_ */
