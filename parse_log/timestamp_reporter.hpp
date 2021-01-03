/*
 * clean_file_writer.hpp
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef TIMESTAMP_REPORTER_HPP_
#define TIMESTAMP_REPORTER_HPP_
#include <iostream>
#include "messages.hpp"
#include "parse_error.hpp"

/**
 * This class reports on jumps in logger timestamps and in gps timestamps
 */
struct timestamp_reporter : public rtlogs::messages_definition
{
    timestamp_reporter(std::ostream &output)
    :last_timestamp(0), last_gps_timestamp(0), output(output), found_gps(false)
    {
    }

    template<typename iterator> void handle(timestamp, iterator begin, iterator end)
    {
        iterator current = begin;
        ++current;
        unsigned long time_value = *current++;
        time_value = (time_value << 8) + *current++;
        time_value = (time_value << 8) + *current++;

        if(last_timestamp && (last_timestamp > time_value || time_value - last_timestamp > 50000))
        {
            output << "log jump: " << last_timestamp << " -> " << time_value;
            if (last_timestamp > time_value)
            {
                output << " -" << last_timestamp - time_value;
            }
            else
            {
                output << " +" << time_value - last_timestamp;
            }
            output << '\n';
        }

        last_timestamp = time_value;
    }

    template <typename iterator> void handle( gps_time_storage, iterator begin, iterator end)
    {
        found_gps = true;
        iterator current = begin;
        ++current;

        // read a 4-byte unsigned big-endian value
        unsigned long time_value = *current++;
        time_value = (time_value << 8) + *current++;
        time_value = (time_value << 8) + *current++;
        time_value = (time_value << 8) + *current++;

        if(last_gps_timestamp && (last_gps_timestamp > time_value || time_value - last_gps_timestamp > 50000))
        {
            output << "gps jump: " << last_gps_timestamp << " -> " << time_value;
            if (last_gps_timestamp > time_value)
            {
                output << " -" << last_gps_timestamp - time_value;
            }
            else
            {
                output << " +" << time_value - last_gps_timestamp;
            }
            output << '\n';
        }

        last_gps_timestamp = time_value;
    }

    template< typename message, typename iterator> void handle( message, iterator, iterator) {}; // ignore everything else.

    [[nodiscard]] bool gps_found( ) const
    {
        return found_gps;
    }

private:
    unsigned long last_timestamp;
    unsigned long last_gps_timestamp;
    bool          found_gps;
    std::ostream  &output;

};


#endif /* TIMESTAMP_REPORTER_HPP_ */
