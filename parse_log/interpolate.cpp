//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "tool_implementation.hpp"
#include "binary_file_writer.hpp"
#include "interpolator.hpp"
#include "logscanner.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iterator>

using std::istreambuf_iterator;


struct interpolate : public rtlogs::input_output_tool
{
public:
    interpolate()
    : input_output_tool{ "interpolate", "interpolated_"}
    {}

protected:
    void run_on_file( const path &from, const path &to) override
    {
        boost::filesystem::ifstream input_file( from, std::ios::binary);
        boost::filesystem::ofstream output_file( to, std::ios::binary);

        using iterator = std::istreambuf_iterator<char>;
        using buffer_type = std::vector<unsigned char>;

        // load the complete file into memory

        const buffer_type buffer{ iterator{input_file}, iterator{}};

        binary_file_writer writer{ output_file};
        interpolator<binary_file_writer> interpolator{ writer, {35, 36, 37}};

        scan_log( interpolator, begin( buffer), end(buffer));
    }
};

interpolate interpolate_tool_instance;
rtlogs::tool_registrar interpolate_tool_registrar( &interpolate_tool_instance);


