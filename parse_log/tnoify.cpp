/*
 * tnoify.cpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: May 13, 2012
 *      Author: danny
 */

#include "timestamp_correction.hpp"
#include "tool_implementation.hpp"
#include "logscanner.hpp"
#include "analogue_channel_table.hpp"
#include "island_removal.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/regex.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <locale>

using namespace std;
using namespace timestamp_correction;
using namespace boost::filesystem;
using namespace boost;

template<typename CharType>
class punctuation_facet : public std::numpunct<CharType>
{
public:

  typedef CharType char_type;

  explicit punctuation_facet(const CharType& decimalPoint) :
    decimalPoint_(decimalPoint), numpunct<CharType>(static_cast<size_t>(0))
  {
  }

  punctuation_facet(const punctuation_facet& rhs) :
    decimalPoint_(rhs.decimalPoint_)
  {
  }

protected:

  ~punctuation_facet() override
  = default;

  CharType do_decimal_point() const override
  {
    return decimalPoint_;
  }

private:

  punctuation_facet& operator=(const punctuation_facet& rhs) = delete;
  const CharType decimalPoint_;
};


struct tnoify_tool : public rtlogs::input_output_tool
{
    tnoify_tool()
            :input_output_tool( "tnoify", "tno_") {};

protected:

    path invent_target_name( const path &source) override
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

    void handle_options( ) override
    {
        string settingsfile;
        if (!get_option( "f", settingsfile, ""))
        {
            throw std::runtime_error( "This tool needs a column definition file. Use the -f command line switch.");
        }

        read_column_file( settingsfile);
        get_option( "p", interval, 10.0);

    }

    void run_on_file( const path &from, const path &to) override
    {
        using buffer_type = std::vector<unsigned char>;

        path csvfile = to;
        if (extension( csvfile) == ".run")
        {
            csvfile = csvfile.parent_path() / path( csvfile.stem().string() + "_.csv");
        }


        boost::filesystem::ifstream input_file( from, std::ios::binary);
        boost::filesystem::ofstream output_file( csvfile, std::ios::binary);

        // set the date/time format for the output file to produce dates like:
        // "01-12-2012 13:01:10"
        using boost::posix_time::time_facet;
        auto *facet(new time_facet("%d-%m-%Y %H:%M:%S"));
        output_file.imbue( std::locale( output_file.getloc(), facet));

        using boost::gregorian::date_facet;
        auto *dfacet(new date_facet("%d-%m-%Y"));
        output_file.imbue( std::locale( output_file.getloc(), dfacet));

        auto *pfacet( new punctuation_facet<char>(','));
        output_file.imbue( std::locale( output_file.getloc(), pfacet));

        // copy the file contents into a buffer
        input_file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( input_file), end;
        const buffer_type buffer( begin, end);

        // first, scan the input for parameters and first values.
        analogue_channel_table table( output_file, interval);

        {
            island_remover<analogue_channel_table> first_remover( table);
            scan_log( first_remover, buffer.begin(), buffer.end());
            first_remover.flush();
        }


        output_file << "Data output van Race Technology opname apparaat\n";
        output_file << "Date Exported: " << boost::gregorian::day_clock::local_day() <<  '\n';
        output_file << "Start Time/Date of exported data: " << table.get_date() << '\n';
        output_file << std::string( 2, '\n');

        // now force a set of columns and their order on this table.
        table.set_columns( columns);
        // now send the data, via the timestamp corrector
        timestamp_correction::time_correction<analogue_channel_table> corrector( table);
        island_remover<timestamp_correction::time_correction<analogue_channel_table> > remover( corrector);
        scan_log( remover, buffer.begin(), buffer.end());
    }

private:
    void read_column_file( const path &p)
    {
        columns.clear();
        boost::filesystem::ifstream input_file( p);
        if (!input_file) throw std::runtime_error( "could not open column definition file: " + p.string());
        static const regex line( R"((\d+):(\d+)\s*=\s*(.*)\s*)");

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





