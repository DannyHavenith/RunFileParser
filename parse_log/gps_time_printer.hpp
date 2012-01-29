/*
 * gps_time_printer.hpp
 *
 *  Created on: Jan 28, 2012
 *      Author: danny
 */

#ifndef GPS_TIME_PRINTER_HPP_
#define GPS_TIME_PRINTER_HPP_


#include <iostream>
#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "messages.hpp"


/**
 * This class handles timestamp messages only. It prints the timestamp for every unique timestamp it receives, together
 * with a count of how often that same value was encountered.
 */
struct gps_timestamp_printer : public rtlogs::messages_definition
{
    gps_timestamp_printer( std::ostream &out)
        :out( out), last_timestamp(0), last_gps_timestamp(0), last_timestamp_at_gps(0) {};

    /// do nothing with most messages.
    void handle( ...)
    {
    };

    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        unsigned long result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        last_timestamp = result;
    }

    template< typename iterator>
    void handle( gps_time_storage, iterator begin, iterator end)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        ++begin; // skip header

        // extract gps timestamp (4-byte, big-endian, unsigned integer)
        unsigned long gps_timestamp = *begin++;
        gps_timestamp = (gps_timestamp << 8) + * begin++;
        gps_timestamp = (gps_timestamp << 8) + * begin++;
        gps_timestamp = (gps_timestamp << 8) + * begin++;

        static const date start( 2012, Jan, 1); //start with a known Sunday, 00:00:00.
        ptime timeval( start, milliseconds(gps_timestamp)); // calculate a day/time given the known Sunday offset.

        // calculate the effective clock rate of the logger clock.
        unsigned long gps_interval = gps_timestamp - last_gps_timestamp;
        unsigned long logger_interval = last_timestamp - last_timestamp_at_gps;
        double rate = (1000.0 * logger_interval) / gps_interval;

        // report to output.
        out << last_timestamp << '\t' << timeval.date().day_of_week() << ' ' << timeval.time_of_day() << '\t' << std::setprecision(5) << rate << '\n';

        last_gps_timestamp = gps_timestamp;
        last_timestamp_at_gps = last_timestamp;
    }

private:
    std::ostream &out;
    unsigned long last_timestamp;
    unsigned long last_gps_timestamp;
    unsigned long last_timestamp_at_gps;
};




#endif /* GPS_TIME_PRINTER_HPP_ */
