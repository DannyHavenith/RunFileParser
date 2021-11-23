//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_CSV_TO_RUN_HPP_
#define PARSE_LOG_CSV_TO_RUN_HPP_

#include "value_encoding.hpp"
#include "messages.hpp"

#include <boost/mpl/at.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <map>
#include <functional>
#include <vector>
#include <string>
#include <numeric>
#include <type_traits>
#include <algorithm>

using boost::mpl::at;

namespace csv
{
    struct column_spec
    {
        int column;
        int header;
        int channel;
    };
    using columns_type = std::vector<column_spec>;
    using record = std::vector<std::string>;

    /**
     * parse a comma separated line of values into a
     * vector of strings.
     *
     * Remove trailing and preceding spaces of each value.
     */
    csv::record parse_line( const std::string &line)
    {
        using namespace boost;

        tokenizer<escaped_list_separator<char> > tok(line);
        csv::record result{ };
        std::transform( tok.begin(), tok.end(), std::back_inserter(result),
            []( const auto &value) { return boost::algorithm::trim_copy(value);});

        return result;
    }

/**
 * Objects of this class read csv-data and translate the values
 * to messages that can then be inserted in a stream.
 *
 * messages are offered to an object of class output.
 */
template< typename output_type>
class csv_to_run
{
public:


    csv_to_run( const csv::columns_type &columns, output_type &output)
    :handlers{ create_handlers( columns)},
     output{ output}
    {
    }

    void handle_row( const csv::record &row)
    {
        for ( const auto &handler : handlers)
        {
            handler(output, row);
        }
    }



private:
    using handler_type = std::function< void ( output_type &, const csv::record &)>;
    using handlers_type = std::vector< handler_type>;

    handlers_type handlers;
    output_type &output;

    /**
     * create_message_writer is a factory that creates a message writer
     * for the given message type.
     *
     * A message writer is a functor object that will retrieve one or more values from
     * a csv record (a line in a csv file) and encode them in a message, to subsequently
     * hand that message to an output message handler.
     *
     */
    template< typename messagetype, typename = void>
    struct create_message_writer
    {
        static handler_type create( int header, int channel, int column)
        {
            return {};
        }
    };

    /**
     * The specialization of message writers for detailed_message or
     * detailed_message_range. These types have a nested type called 'parts'
     */
    template< typename messagetype>
    struct create_message_writer< messagetype, std::void_t<typename messagetype::parts>>
    {
        /**
         * This returns an actual handler. A handler will extract a
         * value out of a csv record and optionally emit a message that is
         * sent to the output.
         */
        static handler_type create( int header, int channel, int column)
        {
            return
                    [ c = context{ header, channel, column}]
                    ( output_type &out, const csv::record &values)
                    {
                        if (not values[c.column].empty())
                        {
                            static constexpr auto size = rtlogs::size<messagetype>::value;
                            unsigned char buffer[size] = { static_cast<unsigned char>(c.header), 0};

                            using parts = typename messagetype::parts;
                            using namespace boost::mpl;
                            auto iterator = buffer + 1;
                            iterator = encode( typename at_c<parts, 0>::type{}, c, values, iterator);
                            iterator = encode( typename at_c<parts, 1>::type{}, c, values, iterator);
                            iterator = encode( typename at_c<parts, 2>::type{}, c, values, iterator);
                            *iterator++ = std::accumulate( buffer, iterator, 0);

                            out.handle( messagetype{}, buffer, iterator);
                        }
                    };
        }
    };

    /**
     * Given a header, a channel and a column number, create a handler (a message creator)
     * that will generate messages of the type indicated by header.
     *
     * If no handler is implemented for the given channel, a default-constructed handler will
     * be returned.
     */
    static handler_type create_handler( int header, int channel, int column)
    {
        handler_type result = {};

        const auto matcher = [&result, header, channel, column]< typename message>( const message &m)
        {
            if constexpr (rtlogs::is_message_range<message>::value)
            {
                if (not result and message::header_begin <= header and header < message::header_end)
                {
                    result = create_message_writer<message>::create( header, channel, column);
                }
            }
            else
            {
                if (not result and message::header == header)
                {
                    result = create_message_writer<message>::create( header, channel, column);
                }
            }
        };

        boost::fusion::for_each( rtlogs::messages_definition::list{}, matcher);

        return result;
    }

    handlers_type create_handlers( const csv::columns_type &columns)
    {
        handlers_type handlers;
        for (const auto column: columns)
        {
            auto handler = create_handler( column.header, column.channel, column.column);
            if ( not handler)
            {
                throw std::runtime_error( "header,channel not supported :" + std::to_string( column.header) + "," + std::to_string( column.channel));
            }
            handlers.emplace_back( std::move(handler));
        }
        return handlers;
    }

};
}

#endif /* PARSE_LOG_CSV_TO_RUN_HPP_ */
