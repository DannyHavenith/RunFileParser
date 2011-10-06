/*
 * clean_file_writer.hpp
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef CLEAN_FILE_WRITER_HPP_
#define CLEAN_FILE_WRITER_HPP_
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>
#include "messages.hpp"
#include "parse_error.hpp"

/**
 * This class simply writes all correctly parsed messages to a file. If there are jumps in the timestamp message
 * larger than some constant (5000), a new file will be opened and subsequent messages will be written to that new file.
 */
struct clean_file_writer : public rtlogs::messages_definition
{
    clean_file_writer( const boost::filesystem::path &base)
    :parent_path( base.parent_path()), base(basename(base)), extension( boost::filesystem::extension( base)),
     last_timestamp(0)
    {
        filename_suffix[0] = 'a';
        filename_suffix[1] = 0;

        open_next_file();
    }

    template<typename iterator>
    void handle( rtlogs::parse_error, iterator begin, iterator end)
    {
        // do nothing with unparsable bytes.
    }

    template< typename iterator>
    void handle( raw_gps, iterator, iterator) {} // ignore.

    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        // write the packet to the current output file.
        std::copy( begin, end, ostreambuf_iterator( output));
    }

    /**
     * handle timer messages. When the timestamp makes a large jump, or becomes lower than the previous value,
     * a new file is opened and output will be delegated to that file.
     */
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        iterator current = begin;
        ++current;
        unsigned long time_value = *current++;
        time_value = (time_value << 8) + * current++;
        time_value = (time_value << 8) + * current++;

        if (last_timestamp && (last_timestamp > time_value || time_value - last_timestamp > 5000))
        {
            std::cerr << "jump: " << last_timestamp << " -> " << time_value << '\n';
            open_next_file();
        }

        // write the packet to the current output file.
        std::copy( begin, end, ostreambuf_iterator( output));

        last_timestamp = time_value;
    }

private:

    /**
     * open a new output file. Files will be typically named "<inputfilebase>'a'.<inputfileext>", "<inputfilebase>'b'.<inputfileext>", etc.,
     * and be placed in the same directory as the input file (whose name was provided as a constructor argument).
     */
    void open_next_file()
    {
        if (output.is_open())
        {
            output.close();
        }
        std::cerr << parent_path / (base + filename_suffix + extension)  << '\n';
        output.open(  parent_path / (base + filename_suffix + extension) , std::ios::binary);
        if (!output.is_open())
        {
            throw std::runtime_error("could not open output file");
        }
        ++filename_suffix[0];

        // write a header to the file
        static const char header[] = {0x98 , 0x1d , 0x00 , 0x00 , 0xc8 , 0x00 , 0x00 , 0x00};
        std::copy( header, header + sizeof header, ostreambuf_iterator( output));
    }

    typedef std::ostreambuf_iterator<char> ostreambuf_iterator;

    const boost::filesystem::path parent_path;
    const std::string           base;
    char                        filename_suffix[2];
    const std::string           extension;
    boost::filesystem::ofstream output;
    unsigned long               last_timestamp;
};


#endif /* CLEAN_FILE_WRITER_HPP_ */
