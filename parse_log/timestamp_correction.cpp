/*
 * timestamp_correction.cpp
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "timestamp_correction.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;

/**
 * General implementation of a tool that takes an input- and an output parameter.
 */
struct from_to_tool : public rtlogs::tool_impl
{
    from_to_tool( const std::string &name, const std::string &prefix)
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

struct timestamp_correction_tool : public from_to_tool
{
    timestamp_correction_tool()
            :from_to_tool( "correct", "corrected_") {};

protected:

   virtual void run_on_file( const path &from, const path &to)
    {
        typedef std::vector<char> buffer_type;


        boost::filesystem::ifstream input_file( from);
        boost::filesystem::ofstream output_file( to);

        // copy the file contents into a buffer
        input_file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( input_file), end;
        const buffer_type buffer( begin, end);

        timestamp_correction::time_correction corrector( output_file);
        scan_log( corrector, buffer.begin(), buffer.end());
    }

} timestamp_correction_tool_instance;

rtlogs::tool_registrar timestamp_correction_tool_registrar( &timestamp_correction_tool_instance);




