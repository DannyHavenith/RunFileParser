//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_CSV_TO_RUN_HPP_
#define PARSE_LOG_CSV_TO_RUN_HPP_

#include "tool_implementation.hpp"

class csv_to_run : public rtlogs::input_output_tool
{
    csv_to_run()
    : input_output_tool{ "fromcsv", "RUN_"}
    {

    }
};

#endif /* PARSE_LOG_CSV_TO_RUN_HPP_ */
