/*
 * histogram_counter.hpp
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef HISTOGRAM_COUNTER_HPP_
#define HISTOGRAM_COUNTER_HPP_
#include <ostream>
#include "messages.hpp"
#include "parse_error.hpp"

struct histogram_counter : public rtlogs::messages_definition
{
    histogram_counter( )
    {
        std::fill( byte_count, byte_count+ (sizeof byte_count/sizeof byte_count[0]), 0);
        std::fill( message_count, message_count+ (sizeof message_count/sizeof message_count[0]), 0);
    }

    template<typename iterator>
    void handle( rtlogs::parse_error, iterator begin, iterator end)
    {
        byte_count[0] += end - begin;
        ++message_count[0];
    }

    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        byte_count[(unsigned char)*begin] += end - begin;
        ++message_count[(unsigned char)*begin];
    }

    void output( std::ostream &out)
    {
        for (int i = 0; i < 256;++i)
        {
            if (message_count[i])
            {
                out << i << '\t' << message_count[i]<< '\t' << byte_count[i] << '\n';
            }
        }
    }

    size_t byte_count[256];
    size_t message_count[256];
};




#endif /* HISTOGRAM_COUNTER_HPP_ */
