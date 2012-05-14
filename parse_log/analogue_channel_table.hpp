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
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/cstdint.hpp>
#include "messages.hpp"

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

    /**
     * Initialize an object of this class with an output stream and a reporting period. If the reporting period is zero, or less than 1/100,
     * a line of output will be emitted at every change in a channel.
     */
    analogue_channel_table( std::ostream &output, double reporting_period_in_seconds = 0.0)
    :last_timestamp(0), output(output),scanning(true), silent_until(0),reporting_period(reporting_period_in_seconds * ticks_per_second)
    {
    }

    /**
     * Set this object in scanning mode, or in non-scanning mode.
     *
     * Objects of this class are are offered events of a log twice: the first time is while the object
     * is in scanning mode. The second time actual output will be generated.
     *
     * After construction, objects of this class are in scanning mode. This means that
     * when input is offered, no output is generated, but an internal table of all encountered event
     * types is built. This table is necessary to output all data in their proper column when this class
     * is used in non-scanning mode.
     *
    */
    void set_scanning( bool new_scanning)
    {
        using boost::lambda::bind;
        using boost::lambda::_1;

        scanning = new_scanning;
        if (!scanning)
        {
            silent_until = 0;
            last_timestamp = 0;

            // output a header line
            output << "timestamp\tchanged\t";
            for (map_type::const_iterator header = values.begin(); header != values.end(); ++ header)
            {
                output << header->first.first << ':' << header->first.second << '\t';
            }
            output << '\n';

            // fill all values with 0.0
            std::for_each( values.begin(), values.end(), boost::lambda::bind( &pair_type::second, _1) = 0.0);
        }
    }

    /// ignore most messages
    void handle(...) {};

    /// store the most recent timestamp
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        size_t result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        last_timestamp = result;
        if (0 != reporting_period) emit_values( key_type(0,0));
    }

    /**
     * Analogue messages each contain one big-endian, fixed point (/1000) unsigned integer of 16 bits
     */
    template<typename iterator>
    void handle( analogue, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const key_type p = std::make_pair( header, 0);

        boost::uint16_t value = ((boost::uint16_t)*begin++) << 8;
        value |= *begin ;

        new_value( p, ((double)value) / 1000.0);
    }

    /**
     * External temperature messages contain multiplexed little-endian,
     * fixed point (/10) signed integers of 16 bits
     */
    template<typename iterator>
    void handle( external_temperature, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const unsigned short index = *begin++;
        const key_type p = std::make_pair( header, index);

        boost::int16_t value = *begin++;
        value |= *begin * 256;

        new_value( p, ((double)value) / 10.0);
    }

    /**
     * External percentage messages contain multiplexed little-endian,
     * fixed point (/10) signed integers of 16 bits
     */
    template<typename iterator>
    void handle( external_percentage, iterator begin, iterator end)
    {
        // it's just like external_temperature
        handle( external_temperature(), begin, end);
    }

    /**
     * External temperature contain multiplexed little-endian, fixed point (/10) unsigned integers of 16 bits
     */
    template<typename iterator>
    void handle( external_frequency, iterator begin, iterator end)
    {
        const unsigned short header = *begin++;
        const unsigned short index = *begin++;
        const key_type p = std::make_pair( header, index);

        boost::uint16_t value = *begin++;
        value += *begin * 256;
        new_value( p, ((double)value)/10.0);
    }

    /**
     * External misc contains multiplexed little-endian, fixed point (/10) unsigned integers of 16 bits.
     */
    template<typename iterator>
    void handle( external_misc, iterator begin, iterator end)
    {
        // it's just the same as external_frequency
        handle( external_frequency(), begin, end);
    }




private:
    typedef std::pair< unsigned short, unsigned short> key_type;
    void emit_values( const key_type &changed)
    {
        if (!scanning && last_timestamp > silent_until)
        {
            // we know now that we have at least one timestamp (since last_timestamp > silent_until >= 0),
            // but maybe our silent_until has not been set for the first time.
            if (0 == silent_until)
            {
                // don't output anything until we've seen at least one reporting period of data.
                silent_until = last_timestamp + reporting_period;
            }
            else
            {
                using boost::lambda::_1;
                using boost::lambda::bind;

                // print all last known values.
                output << last_timestamp << '\t' << changed.first << ':' << changed.second << '\t';
                std::for_each( values.begin(), values.end(), output << boost::lambda::bind( &pair_type::second, _1) << '\t');
                output << '\n';

                // remain silent for a while (if rate > 0)
                silent_until += reporting_period;
            }
        }
    }
    void new_value( const key_type &key, double value)
    {
        values[key] = value;
        emit_values( key);
    }
    typedef std::map<key_type, double> map_type;
    typedef map_type::value_type  pair_type;

    map_type       values;
    size_t         last_timestamp;
    std::ostream   &output;
    bool           scanning;
    size_t         silent_until;     ///< remain silent until the last_timestamp reaches this value.
    int            reporting_period; ///< maximum rate at which to output values. zero by default, which means output at every variable change.
    static const int ticks_per_second = 100;

};


#endif /* ANALOGUE_CHANNEL_TABLE_HPP_ */
