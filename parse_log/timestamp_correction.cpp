/*
 * timestamp_correction.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 25, 2012
 *      Author: danny
 */

#include "timestamp_correction.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"
#include "island_removal.hpp"
#include "binary_file_writer.hpp"
#include "register_tool.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <vector>

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;


struct timestamp_correction_tool : public rtlogs::input_output_tool
{
    timestamp_correction_tool()
            :input_output_tool( "correct", "corrected_") {};

protected:

   void run_on_file( const path &from, const path &to) override
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

};

template void register_tool<timestamp_correction_tool>();

