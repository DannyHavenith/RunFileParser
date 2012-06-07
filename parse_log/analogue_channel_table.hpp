/*
 * analogue_channel_table.hpp
 *
 *  Created on: Oct 5, 2011
 *      Author: danny
 */

#ifndef ANALOGUE_CHANNEL_TABLE_HPP_
#define ANALOGUE_CHANNEL_TABLE_HPP_

#include <map>
#include <algorithm>
#include <utility> // for std::pair, std::make_pair
#include <string>
#include <vector>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/cstdint.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/lexical_cast.hpp"

#include "messages.hpp"
#include "bytes_to_numbers.hpp"

/**
 * This class generates a tab-separated text file with all floating point values found in the file.
 *
 * This class should be used to traverse a file twice: the first time in scanning mode. This is when the class
 * will create a list of all channels and assign them a column number. Then the user of this class should use the set_scanning
 * function to switch this class to output mode and offer the same data again. Now the data will be output using the columns as
 * defined in the scanning phase.
 *
 * This class can also use a 'reporting period' if this is set to a value 'n'(other than zero) it will only output a line after every n
 * seconds of run file data. Any remaining seconds at the end of the file will not be reported.
 *
 * If the reporting period is zero, or less than 1/100, this class will output a line of data whenever a channel changes value.
 */
class analogue_channel_table : public rtlogs::messages_definition
{
public:

    typedef std::pair<unsigned short, unsigned short>       channel_index;
    typedef std::vector<
            std::pair<channel_index, std::string>
    > column_info;

    analogue_channel_table( std::ostream& output,
            double reporting_period_in_seconds = 0.0) :
            last_timestamp(0), first_timestamp(0), output(output), scanning(true), silent_until(0), reporting_period(
                    reporting_period_in_seconds * ticks_per_second), first_date(boost::posix_time::not_a_date_time), separator(";")
    {
    }

    boost::posix_time::ptime get_date() const
    {
        return first_date;
    }

    void reset_values()
    {
        using boost::lambda::_1;
        silent_until    = 0;
        last_timestamp  = 0;
        first_timestamp = 0;
        std::for_each(values.begin(), values.end(),
                boost::lambda::bind(&pair_type::second, _1) = 0.0);
    }

    void set_scanning( bool new_scanning)
    {
        using boost::lambda::bind;
        using boost::lambda::_1;
        scanning = new_scanning;
        if (!scanning)
        {
            headers.clear();
            reset_values();
            for (
                    map_type::const_iterator valueIt = values.begin();
                    valueIt != values.end();
                    ++valueIt
                 )
            {
                headers.push_back(
                        std::make_pair(
                                boost::lexical_cast<std::string>( valueIt->first.first) + ":" + boost::lexical_cast<std::string>( valueIt->first.second)
                                , valueIt
                        )
                );
            }
            print_header();
            first_timestamp = 0;
        }
    }

    /**
     * Use this function if the columns are known beforehand. This sets the columns that will be output and their order.
     * columns that aren't mentioned here will not be printed.
     */
    void set_columns( column_info &columns)
    {
        headers.clear();
        values = first_values;
        for (
                column_info::const_iterator col = columns.begin();
                col != columns.end();
                ++col
             )
        {
            map_type::iterator valueIt = values.insert( std::make_pair(col->first, 0.0)).first;
            headers.push_back( std::make_pair( col->second, valueIt));
        }
        print_header();
        scanning = false;
        silent_until    = 0;
        last_timestamp  = 0;
        first_timestamp = 0;
     }

    void handle( ...)
    {
    }

    template<typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        size_t result = *begin++;
        result = (result << 8) + *begin++;
        result = (result << 8) + *begin++;
        last_timestamp = result;
        if (!first_timestamp) first_timestamp = result;
        if (0 != reporting_period)
            emit_values();
    }

    template< typename iterator>
    void handle( gps_position, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        using bytes_to_numbers::get_big_endian;
        boost::int32_t longitude = get_big_endian<4, boost::int32_t>( begin);
        boost::int32_t latitude = get_big_endian<4, boost::int32_t>( begin + 4);
        boost::int32_t accuracy = get_big_endian<4, boost::int32_t>( begin + 8);
        new_value( header, 0, static_cast<double>(longitude) * 0.0000001);
        new_value( header, 1, static_cast<double>(latitude) * 0.0000001);
        new_value( header, 2, static_cast<double>(accuracy) / 1000.0);

    }

    template<typename iterator>
    void handle( accelerations, iterator begin, iterator end)
    {
        using namespace boost;
        using bytes_to_numbers::get_big_endian;

        const unsigned short header = *begin++;
        double lateral = get_big_endian< 2, int16_t>( begin);
        double longitudinal = get_big_endian< 2, int16_t>( begin + 2);
        new_value( header, 0, lateral / 256.0);
        new_value( header, 1, longitudinal / 256.0);
    }

    template <typename iterator>
    void handle( gps_raw_speed, iterator begin, iterator end)
    {
        using namespace boost;
        using bytes_to_numbers::get_big_endian;
        const unsigned short header = *begin++;

        // in cm/s
        double speed = get_big_endian<4, uint32_t>( begin);
        new_value( header, 0, speed * (3.6 / 100.0));

    }
    /**
     * Date values are handled by this class, but only to find the first reported date in the file.
     */
    template<typename iterator>
    void handle( date_storage, iterator begin, iterator end)
    {
        typedef boost::uint8_t byte;
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        if ( first_date == not_a_date_time)
        {
            byte header  = *begin++;
            byte s = *begin++;
            byte m = *begin++;
            byte h   = *begin++;
            byte day     = *begin++;
            byte month   = *begin++;
            boost::uint16_t year= bytes_to_numbers::get_big_endian<2, uint16_t>(begin);
            try
            {
            date d( year, month, day);
            ptime result( d, hours(h) + minutes(m) +seconds(s));
            first_date = result;
            }
            catch (std::exception&)
            {
                  // intentionally left blank. just don't assign to first date in case of trouble.
            }

            byte offset = begin[2];

        }
    }

    template< typename iterator>
    void handle( gps_time_storage, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        using bytes_to_numbers::get_big_endian;
        boost::uint32_t timestamp = get_big_endian<4, boost::uint32_t>( begin);
        new_value( header, 0, static_cast<double>(timestamp)/1000);
    }

    template<typename iterator>
    void handle( analogue, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        boost::uint16_t value = ((boost::uint16_t)((*begin++))) << 8;
        value |= *begin;
        new_value(header, 0, ((double) ((value))) / 1000.0);
    }

    template<typename iterator>
    void handle( external_temperature, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const unsigned short index = *begin++;
        boost::int16_t value = *begin++;
        value |= *begin * 256;
        new_value(header, index, ((double) ((value))) / 10.0);
    }

    template<typename iterator>
    void handle( external_percentage, iterator begin, iterator end)
    {
        handle(external_temperature(), begin, end);
    }

    template<typename iterator>
    void handle( external_frequency, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const unsigned short index = *begin++;
        boost::uint16_t value = *begin++;
        value += *begin * 256;
        new_value(header, index, ((double) ((value))) / 10.0);
    }

    template<typename iterator>
    void handle( external_misc, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const unsigned short index = *begin++;
        boost::uint16_t value = *begin++;
        value += *begin * 256;
        new_value(header, index, ((double) ((value))) / 100.0);
    }

private:

    void print_header()
    {
        output << "time [s]";
        for (
                header_vector::const_iterator header = headers.begin();
                header != headers.end();
                ++header
             )
        {
            output << separator << header->first;
        }
        output << '\n';
    }

    void emit_values()
    {
        if (!scanning && last_timestamp > silent_until)
        {
            if (0 == silent_until)
            {
                silent_until = last_timestamp + reporting_period;
                first_timestamp = last_timestamp;
            }
            else
            {
                // print all values, in the order determined by the header array.
                output << static_cast<double>(silent_until - first_timestamp - reporting_period)/100;
                for (
                        header_vector::const_iterator header = headers.begin();
                        header != headers.end();
                        ++header
                     )
                {
                    output << separator << header->second->second;
                }
                output << '\n';
                silent_until += reporting_period;
            }
        }

    }

    void new_value( unsigned short channel, unsigned short index, double value)
    {
        const channel_index key( channel, index);
        map_type::iterator it = first_values.find( key);
        if (it == first_values.end())
        {
            first_values[key] = value;
        }
        values[key] = value;
        emit_values();
    }

    typedef std::map<channel_index, double> map_type;
    typedef std::vector< std::pair<std::string, map_type::const_iterator> > header_vector;
    typedef map_type::value_type pair_type;

    map_type        values;
    map_type        first_values;
    header_vector   headers;
    size_t          last_timestamp;
    size_t          first_timestamp;
    std::ostream&   output;
    bool            scanning;
    size_t          silent_until;
    int             reporting_period;
    static const int ticks_per_second = 100;
    boost::posix_time::ptime first_date;
    const std::string separator;
};


#endif /* ANALOGUE_CHANNEL_TABLE_HPP_ */
