/*
 * tnoify.cpp
 *
 *  Created on: May 13, 2012
 *      Author: danny
 */

#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "timestamp_correction.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"
#include "analogue_channel_table.hpp"

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;


struct tnoify_tool : public rtlogs::input_output_tool
{
    tnoify_tool()
            :input_output_tool( "tnoify", "tno_") {};

protected:

   virtual void run_on_file( const path &from, const path &to)
    {
        typedef std::vector<unsigned char> buffer_type;


        path csvpath = to;
        csvpath.replace_extension(".csv");
        boost::filesystem::ifstream input_file( from, std::ios::binary);
        boost::filesystem::ofstream output_file( csvpath, std::ios::binary);

        // copy the file contents into a buffer
        input_file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( input_file), end;
        const buffer_type buffer( begin, end);

        // first, scan the input for parameters.
        analogue_channel_table table( output_file, 10) ;
        scan_log( table, buffer.begin(), buffer.end());
        table.set_scanning(false);

        // now send the data again, but this time through the timestamp corrector
        timestamp_correction::time_correction<analogue_channel_table> corrector( table);
        scan_log( corrector, buffer.begin(), buffer.end());
    }

} tnoify_tool_instance;

rtlogs::tool_registrar tnoify_tool_registrar( &tnoify_tool_instance);





