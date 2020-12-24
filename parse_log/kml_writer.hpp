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
#include <ostream>
#include "messages.hpp"
#include <boost/cstdint.hpp>

/**
 * This class handles gps_position messages only and will output a kml (google earth-) formatted
 * text containing a single trace of those gps_position messages.
 */
struct kml_writer : public rtlogs::messages_definition
{
    kml_writer( std::ostream &out)
       :out(out)
    {
        out << prolog();
        out.precision(8);
    }

    /**
     * ignore all messages that are not of type gps_position
     */
    void handle(...){};

    template< typename iterator>
    void handle( gps_position, iterator begin, iterator end)
    {
        ++begin;

        typedef boost::int32_t long_type;
        long_type longitude = *begin++;
        longitude = (longitude << 8) + *begin++;
        longitude = (longitude << 8) + *begin++;
        longitude = (longitude << 8) + *begin++;
        double longitude_double = longitude/10000000.0;

        long_type latitude = *begin++;
        latitude = (latitude << 8) + *begin++;
        latitude = (latitude << 8) + *begin++;
        latitude = (latitude << 8) + *begin++;
        double latitude_double = latitude/10000000.0;

        out <<  "        " << longitude_double << ',' << latitude_double << ",0.0\n";
    }

    ~kml_writer()
    {
        out << epilog();
    }

private:
    static const char *prolog()
    {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<kml xmlns=\"http://earth.google.com/kml/2.2\">\n"
            "<Placemark>\n"
            "    <name>Path255</name>\n"
            "    <Style>\n"
            "        <LineStyle>\n"
            "            <color>ff0000ff</color>\n"
            "            <width>3.1</width>\n"
            "        </LineStyle>\n"
            "    </Style>\n"
            "    <LineString>\n"
            "        <tessellate>1</tessellate>\n"
            "        <coordinates>\n"
            ;
    }

    static const char *epilog()
    {
        return
            "        </coordinates>\n"
            "    </LineString>\n"
            "</Placemark>\n"
            "</kml>\n"
            ;
    }

    std::ostream &out;
};



#endif /* KML_WRITER_HPP_ */
