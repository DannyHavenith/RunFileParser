/*
 * bytes_to_numbers.hpp
 * Copyright (C) 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Feb 20, 2012
 *      Author: danny
 */

#ifndef BYTES_TO_NUMBERS_HPP_
#define BYTES_TO_NUMBERS_HPP_
#include <cstdint>
namespace bytes_to_numbers
{
    template< std::size_t size, typename T, typename iterator>
    struct big_endian_extractor
    {
        static T extract( iterator &begin)
        {
            // note: this relies on the extract function to increase the iterator.
            T result = big_endian_extractor< size -1, T, iterator>::extract( begin);
            return (result << 8) | (*begin++ & 0xff);
        }
    };

    template< typename T, typename iterator>
    struct big_endian_extractor< 1, T, iterator>
    {
        static T extract( iterator &begin)
        {
            return (unsigned char)*begin++;
        }
    };

    template< std::size_t size, typename T, typename iterator>
    struct little_endian_extractor
    {
        static T extract( iterator &begin)
        {
            // note: this relies on the extract function to increase the iterator.

            T result = (*begin++ & 0xff);
            return result | little_endian_extractor< size -1, T, iterator>::extract( begin) << 8;
        }
    };

    template< typename T, typename iterator>
    struct little_endian_extractor< 1, T, iterator>
    {
        static T extract( iterator &begin)
        {
            return (unsigned char)*begin++;
        }
    };


    template< typename T, typename iterator>
    T get_big_endian( iterator begin)
    {
        return big_endian_extractor<sizeof( T), T, iterator>::extract( begin);
    }

    template< typename iterator>
    unsigned int get_big_endian_u3( iterator begin)
    {
        return big_endian_extractor< 3, unsigned int, iterator>::extract( begin);
    }

    template< typename T, typename iterator>
    T get_little_endian( iterator begin)
    {
        return little_endian_extractor< sizeof( T), T, iterator>::extract( begin);
    }
}

#endif /* BYTES_TO_NUMBERS_HPP_ */
