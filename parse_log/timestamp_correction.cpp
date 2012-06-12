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
#include "island_removal.hpp"

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;


struct timestamp_correction_tool : public rtlogs::input_output_tool
{
    timestamp_correction_tool()
            :input_output_tool( "correct", "corrected_") {};

protected:

   virtual void run_on_file( const path &from, const path &to)
    {
        typedef std::vector<unsigned char> buffer_type;


        boost::filesystem::ifstream input_file( from, std::ios::binary);
        boost::filesystem::ofstream output_file( to, std::ios::binary);



        // copy the file contents into a buffer
        input_file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( input_file), end;
        const buffer_type buffer( begin, end);

        binary_file_writer writer( output_file);
        timestamp_correction::time_correction<binary_file_writer> corrector( writer);
        island_remover<timestamp_correction::time_correction<binary_file_writer> > remover( corrector);
        scan_log( remover, buffer.begin(), buffer.end());
    }

} timestamp_correction_tool_instance;

rtlogs::tool_registrar timestamp_correction_tool_registrar( &timestamp_correction_tool_instance);




