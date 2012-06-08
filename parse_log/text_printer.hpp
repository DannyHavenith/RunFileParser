/*
 * text_printer.hpp
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef TEXT_PRINTER_HPP_
#define TEXT_PRINTER_HPP_
#include <ostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "bytes_to_numbers.hpp"
#include "messages.hpp"

/**
 * This class writes the message out in a csv-format (tab-separated, actually).
 */
struct text_printer : public rtlogs::messages_definition
{
    text_printer( std::ostream &out)
                    :out(out) {}

    void print_value(...){}; // do nothing for most values.

    template< typename iterator>
    void print_value( timestamp, iterator begin, iterator end)
    {

        unsigned long value = bytes_to_numbers::get_big_endian_u3( ++begin);
        out << "\t(" << value << ")";
    }

    /// output the gps time contents to output stream.
    template< typename iterator>
    void print_value( gps_time_storage, iterator begin, iterator end)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        // get the numerical value.
        unsigned long gps_timestamp = bytes_to_numbers::get_big_endian<boost::uint32_t>(++begin);

        static const date epoch( 2012, Jan, 1); //start with a known Sunday, 00:00:00.
        ptime timeval( epoch, milliseconds(gps_timestamp)); // calculate a day/time given the known Sunday offset.

        out << '\t' << '(' << timeval.date().day_of_week() << ' ' << timeval.time_of_day() << ")";

    }

    template< typename message_type, typename iterator>
    void handle( message_type, iterator begin, iterator end)
    {
        iterator current = begin;
        out << message_type::description();
        while (current != end)
        {
            out << '\t' << (int)*current;
            ++current;
        }

        print_value( message_type(), begin, end);
        out << '\n';
    }

private:
    std::ostream &out;
};


#endif /* TEXT_PRINTER_HPP_ */
