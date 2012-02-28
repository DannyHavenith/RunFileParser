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

struct timestamp_correction_tool : public rtlogs::tool_impl
{
    timestamp_correction_tool()
            :tool_impl( "correct", "<source directory> <target directory>") {};

    virtual int run( int argc, char *argv[] )
    {
        if (argc != 2)
        {
            throw_usage_error();
        }

        if (!exists( argv[0]))
        {
            throw std::runtime_error( argv[0] + std::string( " does not exist"));
        }

        if (is_directory( argv[0]))
        {
            correct_directories( argv[0], argv[1]);
        }
        else
        {
            path to( argv[1]);
            path from( argv[0]);
            if (is_directory( to))
            {
                to /= from.filename();
            }
            correct_file( from, to);
        }
        return 0;
    }

private:

    static void correct_directories( const path &from, const path &to)
    {
        typedef std::vector<path> path_vector;
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
                    correct_file( i->path(), to / i->path().filename());
                }
            }
        }

        for (path_vector::const_iterator i = subdirs.begin(); i != subdirs.end();++i)
        {
            correct_directories( from / *i, to / i->filename());
        }

    }

    static void correct_file( const path &from, const path &to)
    {
        typedef std::vector<char> buffer_type;

        // make sure the target directory exists
        create_directories( to.parent_path());

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




