/*
 * kml_writer.hpp
  * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef KML_WRITER_HPP_
#define KML_WRITER_HPP_
#include "messages.hpp"
#include "bytes_to_numbers.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

#include <ostream>
#include <string>
#include <iterator>
#include <boost/cstdint.hpp>
#include <vector>


/**
 * This class looks for specific events and logs the gps position and other data on such events
 */
struct data_at_event : public rtlogs::messages_definition
{
    data_at_event(
            const std::string &sourceName,
            std::ostream &out,
            bool emit_header = true)
       :sourceName( sourceName),
        out(out)
    {
        if (emit_header)
        {
            out << prolog();
        }
        out.precision(8);
    }

    /**
     * ignore all messages that are not of any of the handled types
     */
    void handle(...){};

    template< typename iterator>
    void handle( gps_position, iterator begin, iterator end)
    {
        typedef boost::int32_t long_type;
        using bytes_to_numbers::get_big_endian;

        ++begin;
        const auto longitude = get_big_endian<long_type>(begin);
        std::advance( begin, 4);
        const auto latitude = get_big_endian< long_type>( begin);

        lastposition = {longitude/10000000.0, latitude/10000000.0};
    }


    /// store the last gps time
    template< typename iterator>
    void handle( gps_time_storage, iterator begin, iterator end)
    {
      using namespace boost::posix_time;
      using namespace boost::gregorian;
      using namespace bytes_to_numbers;

      ++begin; // skip header

      // extract gps timestamp (4-byte, big-endian, unsigned integer)
      const auto gps_timestamp = get_big_endian<uint32_t>( begin);
      // gps_timestamp now contains the number of milliseconds since 00:00:00 on Sunday.

      static const date start( 2012, Jan, 1); //start with a known Sunday, 00:00:00.
      last_gps_time = ptime{ start, milliseconds(gps_timestamp)}; // calculate a day/time given the known Sunday offset.

    }

    /// remember the value of the last timestamp encountered.
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        unsigned long result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        last_timestamp = result;

        if (first_timestamp == 0)
        {
            first_timestamp = result;
        }
    }

    template< typename iterator>
    void handle( precalc_distance_data, iterator begin, iterator end)
    {
        ++begin;
        last_distance = bytes_to_numbers::get_big_endian<uint32_t>(begin)/1000.0;

    }

    template< typename iterator>
    void handle( high_res_timer, iterator begin, iterator end)
    {
        if (end-begin > 2 and *(begin+1) == 64)
        {
            output_line( *(begin+1));
        }
    }

//    ~event_position()
//    {
//        out << epilog();
//    }

private:
    std::string prolog()
    {
        std::string prolog{"run"};
        for ( const auto field : { "type", "distance", "time", "reltime", "longitude", "latitude"})
        {
            prolog = prolog + separator + field;
        }
        return prolog + '\n';
    }

    void output_line( unsigned int event_type)
    {
        out
            << sourceName << separator
            << event_type << separator
            << last_distance << separator
            << last_gps_time.time_of_day() << separator
            << (last_timestamp - first_timestamp)/100.0 << separator
            << lastposition.longitude << separator
            << lastposition.latitude
            << '\n'
            ;
    }


    std::string epilog()
    {
        return "";
    }

    struct position
    {
        double longitude;
        double latitude;
    };

    position lastposition = {};
    std::string sourceName;
    std::ostream &out;
    std::string separator = ",";
    boost::posix_time::ptime last_gps_time;
    double last_distance = 0.0;
    unsigned long last_timestamp = 0;
    unsigned long first_timestamp = 0;
};



#endif /* KML_WRITER_HPP_ */
