//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "csv_columns.hpp"

#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <stdexcept>

/**
 * Read a text file that descripes the relation between column
 * names in a csv file and the channels in a run file. This can be
 * used both for csv input and output.
 */
column_info read_column_file( const boost::filesystem::path &p)
{
    column_info columns;

    boost::filesystem::ifstream input_file( p);
    if (!input_file)
    {
        throw std::runtime_error(
                "could not open column definition file: " + p.string());
    }

    static const boost::regex line( R"((\d+):(\d+)\s*=\s*(.*)\s*)");
    std::string buffer;
    while (std::getline( input_file, buffer))
    {
        boost::smatch match;
        if (regex_match( buffer, match, line))
        {
            // the first two numbers are the channel and channel-index.
            const channel_index key{
                    stoul( match[1]),
                    stoul( match[2])};
            columns.push_back( {key, match[3]});
        }
    }

    return columns;
}
