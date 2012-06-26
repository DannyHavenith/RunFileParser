/*
 * timestamp_printer.hpp
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef TIMESTAMP_PRINTER_HPP_
#define TIMESTAMP_PRINTER_HPP_
#include <ostream>

#include "messages.hpp"

/**
 * This class handles timestamp messages only. It prints the timestamp for every unique timestamp it receives, together
 * with a count of how often that same value was encountered.
 */
struct timestamp_printer : public rtlogs::messages_definition
{
    timestamp_printer( std::ostream &out)
        :out( out), last_timestamp(0), first_timestamp(0){};

    /// do nothing with most messages.
    void handle( ...)
    {
    };

    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        unsigned long result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        if (first_timestamp == 0)
        {
            first_timestamp = result;
        }

        out << result << '\t' << result - last_timestamp << '\n';
        last_timestamp = result;
    }

    void flush()
    {
        out << "time span: " << last_timestamp - first_timestamp << '\n';
    }

private:
    std::ostream &out;

    unsigned long last_timestamp;
    unsigned long first_timestamp;
};



#endif /* TIMESTAMP_PRINTER_HPP_ */
