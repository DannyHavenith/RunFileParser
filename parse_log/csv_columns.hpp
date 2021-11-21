//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_CSV_COLUMNS_HPP_
#define PARSE_LOG_CSV_COLUMNS_HPP_

#include <vector>
#include <utility>
#include <string>

#include <boost/filesystem/path.hpp>

using channel_index = std::pair<unsigned short, unsigned short>;
using column_info = std::vector<std::pair<channel_index, std::string>>;

column_info read_column_file( const boost::filesystem::path &p);







#endif /* PARSE_LOG_CSV_COLUMNS_HPP_ */
