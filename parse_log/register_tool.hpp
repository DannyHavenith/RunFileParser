//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_REGISTER_TOOL_HPP_
#define PARSE_LOG_REGISTER_TOOL_HPP_

#include "toolregistry.hpp"

template <typename Tool>
void register_tool()
{
    static Tool instance;
    rtlogs::tool_registry::instance().register_tool( &instance);
}



#endif /* PARSE_LOG_REGISTER_TOOL_HPP_ */
