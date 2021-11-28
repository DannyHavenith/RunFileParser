//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "tool_implementation.hpp"

#include <boost/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>



namespace rtlogs
{

    /**
     * Run the single file operation on each of the input arguments
     */
    int single_file_tool::run( int argc, char *argv[])
    {
        using namespace std;
        if (!argc)
        {
            throw_usage_error();
        }
        while (argc)
        {
            // open the file
            ifstream file( argv[0], ios_base::binary);
            if (!file)
            {
                throw runtime_error(
                        string( "could not open file: ") + argv[0]);
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

    /**
     * Analyze the command line arguments and run the tool on each input file
     * given on the command line
     *
     * If two arguments are given, the first is interpreted as an input file name
     * and the second as an output file name
     *
     * If more than two arguments are provided and the last given argument does not
     * match an existing file name or matches an existing directory, then all other
     * arguments are interpreted as input files and output files will be created in
     * a directory named by the last argument.
     *
     * If more than two arguments are provided and the last argument is also an
     * existing file, or if only one argument is given, all arguments are interpreted
     * as input files and the tool will invent an appropriate name for the output file.
     *
     * The actual tool will then be run for each input-output file pair.
     */
    int input_output_tool::run( int argc, char *argv[])
    {
        std::vector<std::string> arguments( argv, argv + argc);

        harvest_options( arguments);
        handle_options();

        if (arguments.empty())
        {
            throw_usage_error();
        }
        if (arguments.size() == 1)
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
            const path destination_dir( arguments[argc - 1]);
            // multiple sources, maybe one target specified.
            if (!exists( destination_dir) || is_directory( destination_dir))
            {
                // multiple sources, one target specified
                for (int arg = 0; arg < argc - 1; ++arg)
                {
                    const path from( arguments[arg]);
                    run_with_source_and_target_name(
                            from,
                            destination_dir / from.filename());
                }
            }
            else
            {
                // multiple sources, no target specified
                for (int arg = 0; arg < argc; ++arg)
                {
                    const path from( arguments[arg]);
                    invent_target_name_and_run( from);
                }
            }
        }

        return 0;
    }

    /**
     * By default, no option handling will be performed.
     */
    void input_output_tool::handle_options()
    {
    }

    /**
     * Construct an appropriate output name for the result from the input name.
     */
    rtlogs::input_output_tool::path input_output_tool::invent_target_name(
        const path &source)
    {
        return source.parent_path() / (prefix + source.filename().string());
    }

    /**
     * Perform simple option parsing.
     *
     * Each argument prefixed with a dash ('-') is interpreted as a single-argument
     * option.
     */
    void input_output_tool::harvest_options( StringVector &arguments)
    {
        auto argument = arguments.begin();
        while (argument != arguments.end())
        {
            if ((*argument)[0] == '-')
            {
                const std::string key = argument->substr( 1);
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

    /**
     * Run the actual tool with a given input name and an invented output name.
     */
    void input_output_tool::invent_target_name_and_run( const path &source)
    {
        run_with_source_and_target_name( source, invent_target_name( source));
    }

    /**
     * Run the actual tool with a specified input- and output file name.
     *
     * The input file name can be a directory, in which case
     * all files in the input directory will be searched for recursively and parsed.
     */
    void input_output_tool::run_with_source_and_target_name(
        const path &source,
        const path &target)
    {
        if (is_directory( source))
        {
            run_on_directories( source, target);
        }
        else
        {
            if (is_directory( target))
            {
                run_on_file( source, target / source.filename());
            }
            else
            {
                run_on_file( source, target);
            }
        }
    }


    /**
     * Run on all .run files in the given "from" directory recursively and
     * run the actual tool on them. Output files will be created in the given
     * "to" directory.
     */
    void input_output_tool::run_on_directories(
        const path &from,
        const path &to)
    {
        using boost::filesystem::directory_iterator;
        bool directory_created = false;
        using path_vector = std::vector<path>;
        path_vector subdirs;
        for (directory_iterator i( from); i != directory_iterator(); ++i)
        {
            if (is_directory( *i))
            {
                subdirs.push_back( i->path().filename());
            }
            else
            {
                if (i->path().extension() == ".run")
                {
                    if (!directory_created)
                    {
                        create_directories( to);
                        directory_created = true;
                    }
                    run_on_file( i->path(), to / i->path().filename());
                }
            }
        }
        for (const auto &subdir : subdirs)
        {
            run_on_directories( from / subdir, to / subdir.filename());
        }
    }

    void tool_impl::throw_usage_error() const
    {
        throw std::runtime_error( "usage: " + name + " " + argument_help);
    }

}

