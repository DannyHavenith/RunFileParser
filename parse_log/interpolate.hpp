//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_INTERPOLATE_HPP_
#define PARSE_LOG_INTERPOLATE_HPP_
#include "tool_implementation.hpp"
#include <boost/filesystem/path.hpp>

struct interpolate : public rtlogs::input_output_tool
{
public:
    interpolate()
    : input_output_tool{ "interpolate", "interpolated_"}
    {}
    void run_on_file( const boost::filesystem::path &from, const boost::filesystem::path &to) override;
protected:
};





#endif /* PARSE_LOG_INTERPOLATE_HPP_ */
