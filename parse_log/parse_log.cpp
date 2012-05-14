// parse_log.cpp : Defines the entry point for the console application.
//

/*
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/size.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>
#include <algorithm>
*/

#include <stdexcept>
#include <string>
#include <iostream>
#include <exception>

#include "toolregistry.hpp"

/*
#include "messages.hpp"
#include "logscanner.hpp"
#include "parse_error.hpp"

#include "clean_file_writer.hpp"
#include "timestamp_printer.hpp"
#include "histogram_counter.hpp"
#include "kml_writer.hpp"
#include "text_printer.hpp"
#include "analogue_channel_table.hpp"
#include "timestamp_reporter.hpp"
#include "gps_time_printer.hpp"
#include "timestamp_correction.hpp"
*/


void error( const std::string &what)
{
    throw std::runtime_error( what);
}

int main(int argc, char* argv[])
{
    using namespace std;
    using namespace rtlogs;

    try
    {
        tool *the_tool = 0;
        if (argc < 2 ||  0 == (the_tool = tool_registry::instance().find_tool( argv[1])))
        {
            cerr << "usage: parse <command> [command options...]\n";
            cerr << "where command is one of:\n";
            rtlogs::tool_registry::instance().print( cerr);
        }
        else
        {
            the_tool->run( ----argc, ++++argv);
        }
    }
    catch (exception &e)
    {
        cerr << "something went wrong: " << e.what() << '\n';
    }

	return 0;
}

