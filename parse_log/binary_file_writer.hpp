//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_BINARY_FILE_WRITER_HPP_
#define PARSE_LOG_BINARY_FILE_WRITER_HPP_

#include <algorithm>
#include <iterator>
#include <iostream>

/**
 * A simple handler class that will just write all parsed messages to a given output file.
 */
class binary_file_writer
{
public:
    binary_file_writer( std::ostream &output)
    :output( output)
    {
        // write a header to the file
       static const unsigned char header[] = {0x98 , 0x1d , 0x00 , 0x00 , 0xc8 , 0x00 , 0x00 , 0x00};
       std::copy( header, header + sizeof header, std::ostreambuf_iterator<char>( output));
    }

    /**
     * all messages are just written verbatim to the output file.
     */
    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        std::copy( begin, end, std::ostreambuf_iterator<char>( output));
    }

private:
    std::ostream &output;
};
#endif /* PARSE_LOG_BINARY_FILE_WRITER_HPP_ */
