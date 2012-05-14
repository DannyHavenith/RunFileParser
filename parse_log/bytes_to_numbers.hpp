/*
 * bytes_to_numbers.hpp
 *
 *  Created on: Feb 20, 2012
 *      Author: danny
 */

#ifndef BYTES_TO_NUMBERS_HPP_
#define BYTES_TO_NUMBERS_HPP_

namespace bytes_to_numbers
{
    template< unsigned int size, typename T, typename iterator>
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

    template< unsigned int size, typename T, typename iterator>
    T get_big_endian( iterator begin)
    {
        return big_endian_extractor<size, T, iterator>::extract( begin);
    }

}

#endif /* BYTES_TO_NUMBERS_HPP_ */
