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
#include <boost/filesystem.hpp>

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
    input_output_tool( const std::string &name, const std::string &prefix)
            : tool_impl( name, "<source>... [destination]"), prefix( prefix) {}


    virtual int run(int argc, char *argv[])
    {
        if(argc == 1)
        {
            // one source, no target specified
            invent_target_name_and_run( argv[0]);
        }
        else if (argc == 2)
        {
            // one source, one target specified
            run_with_source_and_target_name( argv[0], argv[1]);
        }
        else
        {
            const path destination_dir( argv[argc-1]);
            // multiple sources, maybe one target specified.

            if (!exists( destination_dir) || is_directory(destination_dir))
            {
                // multiple sources, one target specified
                for (int arg = 0; arg < argc - 1; ++ arg)
                {
                    const path from( argv[arg]);
                    run_with_source_and_target_name( from, destination_dir/ from.filename());
                }
            }
            else
            {
                // multiple sources, no target specified
                for (int arg = 0; arg < argc; ++ arg)
                {
                    const path from( argv[arg]);
                    invent_target_name_and_run( from);
                }

            }
        }


        return 0;
    }

protected:
    typedef boost::filesystem::path path;
    virtual void run_on_file(const path & from, const path & to) =0;

private:
    void invent_target_name_and_run(const path & source)
    {
        const path to(source.parent_path() / ( prefix + source.filename().string()));
        run_with_source_and_target_name( source, to);
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
        typedef std::vector<path> path_vector;
        path_vector subdirs;
        for(directory_iterator i(from);i != directory_iterator();++i){
            if(is_directory(*i)){
                subdirs.push_back(i->path().filename());
            }else{
                if(i->path().extension() == ".run"){
                    if(!directory_created){
                        create_directories( to);
                        directory_created = true;
                    }
                    run_on_file( i->path(), to / i->path().filename());
                }
            }
        }

        for (path_vector::const_iterator i = subdirs.begin(); i != subdirs.end();++i)
        {
            run_on_directories( from / *i, to / i->filename());
        }

    }

private:
    const std::string prefix;

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
