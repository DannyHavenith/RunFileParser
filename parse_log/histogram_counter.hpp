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

struct histogram_counter : public rtlogs::messages_definition
{
    histogram_counter()
    {
        std::fill( table, table+ (sizeof table/sizeof table[0]), 0);
    }

    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator)
    {
        ++table[(unsigned char)*begin];
    }

    void output( std::ostream &out)
    {
        for (int i = 0; i < 256;++i)
        {
            out << i << '\t' << table[i] << '\n';
        }
    }

    unsigned long table[256];
};




#endif /* HISTOGRAM_COUNTER_HPP_ */
