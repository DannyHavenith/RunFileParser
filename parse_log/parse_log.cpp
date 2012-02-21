// parse_log.cpp : Defines the entry point for the console application.
//

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/size.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>

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

using namespace rtlogs;

void error( const std::string &what)
{
    throw std::runtime_error( what);
}

int main(int argc, char* argv[])
{
    using namespace std;
    typedef std::vector<unsigned char> buffer_type;

    try
    {
        if (argc < 2)
        {
            error("specify a file name");
        }

        // open the log file
        ifstream file( argv[1], ios_base::binary);
        if (!file)
        {
            error( string("could not open file: ") + argv[1]);
        }

        // copy the file contents into a buffer
        file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( file), end;
        const buffer_type buffer( begin, end);

        if (argc == 3)
        {
            if (argv[2] == string("timestamps"))
            {
                // now scan the log bytes in the buffer.
                timestamp_printer printer(cout);
                scan_log( printer, buffer.begin(), buffer.end());
                // cleanup
                printer.flush();
            }
            else if (argv[2] == string("txt"))
            {
                text_printer printer( cout);
                scan_log( printer, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("clean"))
            {
                clean_file_writer writer( argv[1]);
                scan_log( writer, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("histogram"))
            {
                histogram_counter counter;
                scan_log( counter, buffer.begin(), buffer.end());
                counter.output( std::cout);
            }
            else if (argv[2] == string("values"))
            {
                analogue_channel_table table(std::cout) ;
                scan_log( table, buffer.begin(), buffer.end());
                table.set_scanning(false);
                scan_log( table, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("report"))
            {
                timestamp_reporter reporter( std::cout);
                scan_log( reporter, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("gpstime"))
            {
                gps_timestamp_printer printer( std::cout, std::cerr, argv[1]);
                scan_log( printer, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("correct"))
            {
                using namespace boost::filesystem;
                path base;
                if (argc>3)
                {
                    base = path(argv[3]);
                }
                else
                {
                    base = path( argv[1]).parent_path() / "corrected";
                }
                create_directories( base);
                path output_path = base / path( argv[1]).filename();
                boost::filesystem::ofstream outputstream( output_path);
                timestamp_correction::time_correction corrector( outputstream);
                scan_log( corrector, buffer.begin(), buffer.end());
            }
            else
            {
                std::cerr << "could not interpret command: " << argv[2] << "\n";
            }
        }
        else
        {
            kml_writer kml( cout);
            scan_log( kml, buffer.begin(), buffer.end());
        }

    }
    catch (exception &e)
    {
        cerr << "something went wrong: " << e.what() << '\n';
    }

	return 0;
}

