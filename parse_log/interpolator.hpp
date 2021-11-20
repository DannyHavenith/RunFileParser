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
#include <numeric> // for std::accumulate


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

    using timestamp_t = std::uint32_t;
    using value_t = std::int16_t;
    using channel_t = unsigned char;

    interpolator(
        output_handler &output,
        channel_t channel)

    :   output{output},
        channel{channel}
    {

    }

    ~interpolator()
    {
        flush();
    }

    /**
     * store unhandled messages in the buffer
     */
    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        store( begin, end);
    }

    /**
     * Make a note of the last time stamp encountered.
     */
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        auto time = bytes_to_numbers::get_big_endian_u3( std::next(begin));
        last_timestamp = time;
        store( begin, end);
    }

    /**
     * When encountering a value that needs interpolating,
     * flush the buffer, inserting interpolated values while doing so.
     */
    template< typename iterator>
    void handle( external_auxiliary, iterator begin, iterator end)
    {
        const unsigned short index = *std::next( begin);

        if (channel != index)
        {
            store( begin, end);
        }
        else
        {
            const auto new_value = bytes_to_numbers::get_little_endian<std::int16_t>(std::next( begin, 2));

            // if this is the first value we see, just flush the buffer.
            if (last_timestamp_with_value == 0)
            {
                flush();
            }
            else
            {
                injector i{ output, channel, last_timestamp_with_value, last_value, last_timestamp, new_value};
                rtlogs::scan_log( i, std::begin(buffer), std::end(buffer));
            }

            buffer.clear();

            last_timestamp_with_value = last_timestamp;
            last_value = new_value;

            output.handle( external_auxiliary{}, begin, end);
        }
    }


private:


    using buffer_type = std::vector<unsigned char>;

    /**
     * Send all data in the buffer to the output and clear the buffer.
     */
    void flush()
    {
        rtlogs::scan_log( output, begin(buffer), end(buffer));
        buffer.clear();
    }

    template< typename iterator>
    void store( iterator begin, iterator end)
    {
        buffer.insert( std::end(buffer), begin, end);
    }

    buffer_type buffer;
    output_handler &output;
    channel_t channel;
    std::int16_t last_value{};
    unsigned int last_timestamp{};
    unsigned int last_timestamp_with_value{};


    class injector : public rtlogs::messages_definition
    {
    public:
        injector(
            output_handler &output,
            channel_t channel,
            timestamp_t first_value_time,
            value_t first_value,
            timestamp_t last_value_time,
            value_t last_value,
            timestamp_t time_increment = 1)
        : output{output},
          channel{channel},
          first_value_time{first_value_time},
          last_value_time{last_value_time},
          next_emit_time{first_value_time + time_increment},
          time_increment{time_increment},
          first_value{first_value},
          last_value{last_value}
        {
        }

        /**
         * regular messages get sent straight to the output.
         */
        template< typename message, typename iterator>
        void handle( message, iterator begin, iterator end)
        {
            output.handle( message{}, begin, end);
        }

        /**
         * When outputting a time stamp message, first inject an interpolated value
         *
         */
        template< typename iterator>
        void handle( timestamp, iterator begin, iterator end)
        {
            const auto time = bytes_to_numbers::get_big_endian_u3( std::next( begin));
            if (time > next_emit_time)
            {
                emit( std::int64_t(last_value - first_value) * (last_seen_timestamp - first_value_time) / (last_value_time - first_value_time) + first_value);
            }
            last_seen_timestamp = time;
            output.handle( timestamp{}, begin, end);
        }

    private:

        /*
         * Emit an extra External Auxiliary message with an interpolated value
         */
        void emit( value_t interpolated_value)
        {
            unsigned char buffer[] = {
                    74,
                    channel,
                    static_cast<unsigned char>(interpolated_value & 0xff),
                    static_cast<unsigned char>(interpolated_value >> 8),
                    0};
            buffer[4] = std::accumulate( std::begin(buffer), std::prev( std::end(buffer)), 0);

            rtlogs::scan_log( output, std::begin( buffer), std::end(buffer));
        }

        output_handler &output;
        channel_t channel;
        timestamp_t first_value_time;
        timestamp_t last_value_time;
        timestamp_t next_emit_time;
        timestamp_t time_increment;
        timestamp_t last_seen_timestamp{};
        value_t first_value;
        value_t last_value;
    };

};



#endif /* PARSE_LOG_INTERPOLATOR_HPP_ */
