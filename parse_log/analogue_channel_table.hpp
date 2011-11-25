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

class analogue_channel_table : public rtlogs::messages_definition
{
public:
    analogue_channel_table( std::ostream &output)
    :last_timestamp(0), output(output),scanning(true)
    {
    }

    /**
     * Set this object in scanning mode, or in non-scanning mode.
     *
     * Objects of this class are are offered events of a log twice: the first time, while the object
     * is in scanning mode. The second time, actual output will be generated.
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
            output << "timestamp\tchanged\t";

            for (map_type::const_iterator header = values.begin(); header != values.end(); ++ header)
            {
                output << header->first.first << ':' << header->first.second << '\t';
            }

            std::for_each( values.begin(), values.end(), bind( &pair_type::second, _1) = 0.0);
            output << '\n';
            last_timestamp = 0;
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
    }

    /**
     * Analogue messages each contain one big-endian, fixed point (/1000) unsigned integer of 16 bits
     */
    template<typename iterator>
    void handle( analogue1, iterator begin, iterator end)
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
        if (!scanning)
        {
            using boost::lambda::_1;
            using boost::lambda::bind;

            output << last_timestamp << '\t' << changed.first << ':' << changed.second << '\t';
            std::for_each( values.begin(), values.end(), output << bind( &pair_type::second, _1) << '\t');
            output << '\n';
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

};


#endif /* ANALOGUE_CHANNEL_TABLE_HPP_ */
