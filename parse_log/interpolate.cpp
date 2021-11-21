//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "interpolate.hpp"

#include "tool_implementation.hpp"
#include "binary_file_writer.hpp"
#include "interpolator.hpp"
#include "logscanner.hpp"
#include "register_tool.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iterator>


using std::istreambuf_iterator;

void interpolate::run_on_file( const path &from, const path &to)
{
    boost::filesystem::ifstream input_file(from, std::ios::binary);
    boost::filesystem::ofstream output_file(to, std::ios::binary);

    // load the complete file into memory
    using iterator = std::istreambuf_iterator<char>;
    using buffer_type = std::vector<unsigned char>;
    const buffer_type buffer{ iterator{input_file}, iterator{}};

    binary_file_writer writer{output_file};
    interpolator<binary_file_writer> interpolator{ writer, 37};
    scan_log(interpolator, begin(buffer), end(buffer));
}

template void register_tool<interpolate>();


