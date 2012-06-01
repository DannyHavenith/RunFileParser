/*
 * tnoify.cpp
 *
 *  Created on: May 13, 2012
 *      Author: danny
 */

#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>
#include "timestamp_correction.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"
#include "analogue_channel_table.hpp"

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;
using namespace boost;


struct tnoify_tool : public rtlogs::input_output_tool
{
    tnoify_tool()
            :input_output_tool( "tnoify", "tno_") {};

protected:

    virtual path invent_target_name( const path &source)
    {
        if (extension( source) == ".run")
        {
            return source.parent_path() / path( source.stem().string() + "_.csv");
        }
        else
        {
            return input_output_tool::invent_target_name( source);
        }
    }

    virtual void handle_options( )
    {
        string settingsfile;
        if (!get_option( "f", settingsfile, ""))
        {
            throw std::runtime_error( "This tool needs a column definition file. Use the -f command line switch.");
        }

        read_column_file( settingsfile);
        get_option( "p", interval, 10.0);

    }

    virtual void run_on_file( const path &from, const path &to)
    {
        typedef std::vector<unsigned char> buffer_type;

        path csvfile = to;
        if (extension( csvfile) == ".run")
        {
            csvfile = csvfile.parent_path() / path( csvfile.stem().string() + "_.csv");
        }


        boost::filesystem::ifstream input_file( from, std::ios::binary);
        boost::filesystem::ofstream output_file( csvfile, std::ios::binary);

        // copy the file contents into a buffer
        input_file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( input_file), end;
        const buffer_type buffer( begin, end);

        // first, scan the input for parameters and first values.
        analogue_channel_table table( output_file, interval);
        scan_log( table, buffer.begin(), buffer.end());


        output_file << "This data is generated using a beta-version of the run-file export tool v0.1. Do not use for production purposes\n";
        output_file << "Date Exported: " << boost::gregorian::day_clock::local_day() <<  '\n';
        output_file << "Start Time/Date of exported data: " << table.get_date() << '\n';
        output_file << std::string( 2, '\n');

        // now force a set of columns and their order on this table.
        table.set_columns( columns);
        // now send the data, via the timestamp corrector
        timestamp_correction::time_correction<analogue_channel_table> corrector( table);
        scan_log( corrector, buffer.begin(), buffer.end());
    }

private:
    void read_column_file( const path &p)
    {
        columns.clear();
        boost::filesystem::ifstream input_file( p);
        if (!input_file) throw std::runtime_error( "could not open column definition file: " + p.string());
        static const regex line( "(\\d+):(\\d+)\\s+=\\s*(.*)\\s*");

        string buffer;
        while ( std::getline( input_file, buffer))
        {
            smatch match;
            if (regex_match( buffer, match, line))
            {
                // the first two numbers are the channel and channel-index.
                analogue_channel_table::channel_index key( lexical_cast<unsigned short>( match[1]),lexical_cast<unsigned short>( match[2]));
                columns.push_back( std::make_pair( key, match[3]));
            }
        }

    }

    analogue_channel_table::column_info columns;
    double interval; // logging interval

} tnoify_tool_instance;

rtlogs::tool_registrar tnoify_tool_registrar( &tnoify_tool_instance);





