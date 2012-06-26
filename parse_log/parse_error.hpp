/*
 * parse_error.hpp
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * this file contains the definition of a parse_error message tag.
 *
 * This is in a separate header file so that both the log scanner and log scanner event handlers (reactors) can use (include) this
 * type without depending on (including) each other.
 *  Created on: Sep 23, 2011
 *      Author: danny
 */

#ifndef PARSE_ERROR_HPP_
#define PARSE_ERROR_HPP_

namespace rtlogs
{
    /**
     *  this semi-message tag is used to signal a block of bytes in the input stream hat
     *  could not be parsed. Whenever such a block is encountered, the parser will call
     *  r.handle( parse_error(), begin_of_block, end_of_block);
     *  on the reactor r.
     */
    struct parse_error
    {
        static const char *description() { return "parse error";}
    };

}


#endif /* PARSE_ERROR_HPP_ */
