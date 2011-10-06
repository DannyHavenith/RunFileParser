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
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "messages.hpp"

class analogue_channel_table : public rtlogs::messages_definition
{
public:
    analogue_channel_table( std::ostream &output)
    :last_timestamp(0), output(output),scanning(true)
    {
    }

    void set_scanning( bool new_scanning)
    {
        using boost::lambda::bind;
        using boost::lambda::_1;

        scanning = new_scanning;
        if (!scanning)
        {
            output << "timestamp\tchanged\t";
            std::for_each( values.begin(), values.end(), output << bind( &pair_type::first, _1) << '\t' );
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

    template<typename iterator>
    void handle( analogue1, iterator begin, iterator end)
    {
        const unsigned short index = *begin++ - 20;
        double value = *begin++ * 256;
        value += *begin;
        value /= 1000;

        values[index] = value;
        emit_values( index);
    }

    template<typename iterator>
    void handle( extended_frequency1, iterator begin, iterator end)
    {
        const unsigned short index = *begin++ - 58 + 32;
        double value = *begin++ * 256;
        value += *begin;
        value /= 1000;

        values[index] = value;
        emit_values( index);
    }

    template<typename iterator>
    void handle( external_temperature, iterator begin, iterator end)
    {
        ++begin;
        const unsigned short index = *begin++ + 36;
        short value = *begin++;
        value += *begin * 256;


        values[index] = ((double)value) / 10.0;
        emit_values( index);
    }

    void emit_values( unsigned short index_changed)
    {
        if (!scanning)
        {
            using boost::lambda::_1;
            using boost::lambda::bind;

            output << last_timestamp << '\t' << index_changed << '\t';
            std::for_each( values.begin(), values.end(), output << bind( &pair_type::second, _1) << '\t');
            output << '\n';
        }
    }

private:
    typedef std::map<int, double> map_type;
    typedef map_type::value_type  pair_type;

    map_type       values;
    size_t         last_timestamp;
    std::ostream   &output;
    bool           scanning;

};


#endif /* ANALOGUE_CHANNEL_TABLE_HPP_ */
