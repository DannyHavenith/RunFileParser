//
//  Copyright (C) 2020 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include "message_names.hpp"

#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>

#include "messages.hpp"
#include <vector>
#include <string>

namespace
{
    using StringTable = std::vector<std::string>;

    struct description_writer
    {
        description_writer( StringTable &table)
        :table{table}
        {
        }

        template<typename message_type>
        auto operator()( message_type) -> decltype( message_type::header)
        {
            table[message_type::header] = message_type::description();
            return message_type::header;
        }

        template<typename message_type>
        auto operator()( message_type) -> decltype( message_type::header_begin)
        {
            for (auto h = message_type::header_begin; h < message_type::header_end; ++h)
            {
                table[h] = std::string{message_type::description()} + " (" + std::to_string(h - message_type::header_begin) + ")";
            }
            return message_type::header_begin;
        }


        StringTable &table;
    };

    StringTable make_message_name_table()
    {
        StringTable names(256);
        boost::fusion::for_each(
            rtlogs::messages_definition::list{},
            description_writer{ names}
            );

        return names;
    }
}

std::string get_message_name( uint8_t header)
{
    static const auto table = make_message_name_table();
    return table[header];
}


