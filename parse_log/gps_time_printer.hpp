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
 * This class prints the contents of gps time channel packets.
 * The packets are printed, one per line in the following format:
 * <last seen timestamp>\t<gps timestamp>\t<average timestamp clockticks per second since previous gps timestamp>
 * Additionally, if the timestamps-per-gps-second average is lower than 50 for more than 50 consecutive gps timestamps this
 * class will output the file name to another output stream (normally std::cerr). The file name will be sent only once.
 */
struct gps_timestamp_printer : public rtlogs::messages_definition
{
    gps_timestamp_printer( std::ostream &out, std::ostream &error, const std::string &filename)
        :out( out), error( error), filename(filename), last_timestamp(0), last_gps_timestamp(0), last_timestamp_at_gps(0), slow_clock_count(0) {};

    /// do nothing with most messages.
    void handle( ...)
    {
    };

    /// remember the value of the last timestamp encountered.
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        unsigned long result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        last_timestamp = result;
    }

    /// output the gps time contents to output stream, calculating and printing average clock ticks.
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

        // slow_clock_count <0 means we're not interested anymore in slow clocks
        if (slow_clock_count >= 0)
        {
            // we're interested, count the number of consecutive slow clocks
            if (rate < slow_clock_treshold)
            {
                if (++slow_clock_count > slow_clock_count_trigger)
                {
                    // print if we've reached a high enough dose of slow clocks.
                    error << filename << '\n';
                    slow_clock_count = -1;
                }
            }
            else
            {
                slow_clock_count = 0;
            }
        }

        // report to output.
        out << last_timestamp << '\t' << timeval.date().day_of_week() << ' ' << timeval.time_of_day() << '\t' << std::setprecision(5) << rate << '\n';

        last_gps_timestamp = gps_timestamp;
        last_timestamp_at_gps = last_timestamp;
    }

private:
    std::ostream    &out;
    std::ostream    &error;
    const std::string filename;

    ///
    int             slow_clock_count;
    unsigned long   last_timestamp;
    unsigned long   last_gps_timestamp;
    unsigned long   last_timestamp_at_gps;

    /// what ticks/s value is considered a slow clock
    static const int slow_clock_treshold = 50;

    /// how many slow clocks before we raise an alarm.
    static const int slow_clock_count_trigger = 10;
};




#endif /* GPS_TIME_PRINTER_HPP_ */
