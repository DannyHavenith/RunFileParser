//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "csv_to_run.hpp"
#include "csv_columns.hpp"
#include "tool_implementation.hpp"
#include "text_printer.hpp"
#include "messages.hpp"
#include "register_tool.hpp"
#include "binary_file_writer.hpp"

#include <boost/filesystem/fstream.hpp>

#include <iostream>


class csv_to_run_tool : public rtlogs::input_output_tool
{
public:
    csv_to_run_tool()
    : input_output_tool{ "fromcsv", "RUN_"}
    {
    }


protected:
    path invent_target_name( const path &source) override
    {
        using boost::algorithm::to_lower_copy;

        if (to_lower_copy( extension( source)) == ".csv")
        {
            return source.parent_path() / path( source.stem().string() + ".RUN");
        }
        else
        {
            return input_output_tool::invent_target_name( source);
        }
    }

    void run_on_file(const boost::filesystem::path &from , const boost::filesystem::path &to ) override
    {
        using namespace boost::filesystem;



        ifstream input{ from};
        ofstream output{ to, std::ios::binary};
        std::string line;

        if (not getline( input, line))
        {
            throw std::runtime_error("could not read header row from file: " + from.string());
        }
        const auto header = csv::parse_line(line);
        const auto columns = set_columns( definitions, header);

        binary_file_writer writer{ output};
        csv::csv_to_run<binary_file_writer> converter{ columns, writer};

        while (getline( input, line))
        {
            converter.handle_row( csv::parse_line(line));
        }
    }

    void handle_options( ) override
    {
        std::string settingsfile;
        if (!get_option( "f", settingsfile, ""))
        {
            throw std::runtime_error( "This tool needs a column definition file. Use the -f command line switch.");
        }

        definitions = read_column_file( settingsfile);
    }

    csv::columns_type set_columns(
        const column_info &definitions,
        const csv::record &header)
    {
        csv::columns_type result;
        using std::end;
        using std::begin;
        for (const auto &def : definitions)
        {
            const auto pos = std::find( begin(header), end(header), def.second);
            if (pos != end( header))
            {
                int column = std::distance( begin(header), pos);
                result.push_back( {column, def.first.first, def.first.second});
            }
        }
        return result;
    }

    column_info definitions;
};

template void register_tool<csv_to_run_tool>();


