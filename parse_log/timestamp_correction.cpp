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

    virtual void run( int argc, char *argv[] )
    {
        if (argc != 2)
        {
            throw_usage_error();
        }

        correct_directories( argv[0], argv[1]);
    }

private:

    static void correct_directories( const path &from, const path &to)
    {
        std::vector<path> subdirs;
        for (directory_iterator i( from); i != directory_iterator; ++i)
        {
            if (is_directory( *i))
            {
                subdirs.push_back( i->path().filename());
            }
            else
            {
                correct_file( i->path(), to / i->path().filename());
            }
        }
    }

    static void correct_file( const path &from, const path &to)
    {
        boost::filesystem::ifstream input_file( from);
        boost::filesystem::ofstream output_file( to);
        timestamp_correction::time_correction corrector( output_file);
    }

} timestamp_correction_tool_instance;

rtlogs::tool_registrar timestamp_correction_tool_registrar( &timestamp_correction_tool_instance);




