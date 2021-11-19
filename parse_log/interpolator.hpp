//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_INTERPOLATOR_HPP_
#define PARSE_LOG_INTERPOLATOR_HPP_
#include "messages.hpp"
#include "bytes_to_numbers.hpp"
#include "logscanner.hpp"

#include <vector>
#include <map>
#include <set>

/**
 * Message handler that makes sure that there is a minimum frequency of
 * aux messages for a given channel. If no message is present at the required time,
 * a message will be injected with an interpolated message
 *
 * messages will only be injected before the last message in the actual data, so if the input
 * stream ends with a long sequence of non aux messages, there will be no injected messages in that stream.
 */
template< typename output_handler>
class interpolator : public rtlogs::messages_definition
{
public:

    interpolator(
        output_handler &output,
        std::set<int> channels)

    :   output{output},
        channels{channels},
        last_timestamp{}
    {

    }

    /**
     * In general send all messages straight to the output.
     */
    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        output.handle( message{}, begin, end);
    }

    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        auto time = bytes_to_numbers::get_big_endian_u3( std::next(begin));
        insert_if_needed();
        last_timestamp = time;
        output.handle( timestamp{}, begin, end);
    }

    template< typename iterator>
    void handle( external_auxiliary, iterator begin, iterator end)
    {
        const unsigned short index = *std::next( begin);

        if (channels.count( index) != 0)
        {
            last_value[index].assign( begin, end);
            timestamps[index] = last_timestamp;
        }
        output.handle( external_auxiliary{}, begin, end);
    }

private:
    using buffer_type = std::vector<unsigned char>;

    template< typename iterator>
    void emit( iterator begin, iterator end)
    {
        rtlogs::scan_log( output, begin, end);
    }

    /**
     * If we haven't emitted values for our channels yet this timestamp and we have some previous
     * value, repeat that value.
     */
    void insert_if_needed()
    {
        for ( const auto channel : channels)
        {
            if ( 0 < timestamps[channel] and timestamps[channel] < last_timestamp)
            {
                const auto &value = last_value[channel];
                emit( value.begin(), value.end());
                timestamps[channel] = last_timestamp; // not strictly necessary, but eases reasoning about invariants.
            }
        }
    }

    output_handler &output;
    std::set<int> channels;
    unsigned int last_timestamp;
    std::map< int, buffer_type> last_value;
    std::map< int, unsigned int> timestamps;
};



#endif /* PARSE_LOG_INTERPOLATOR_HPP_ */
