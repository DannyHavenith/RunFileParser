//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef PARSE_LOG_VALUE_ENCODING_HPP_
#define PARSE_LOG_VALUE_ENCODING_HPP_
#include "message.hpp"
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/endian/conversion.hpp>

/**
 * translate from string to a value of the requested type
 */
template< typename cooked_value_type>
cooked_value_type to_cooked_type( const std::string &value)
{
    // not the fastest way, but easiest to write
    try
    {
        return boost::lexical_cast< cooked_value_type>( value);
    }
    catch (std::exception &)
    {
        throw std::runtime_error( "could not convert this to a number:'" + value + "'");
    }
}

/**
 * Generally the raw type is the same as the cooked type,
 * so just return the cooked value.
 */
template< typename type, typename cooked_value_type>
cooked_value_type to_raw_type( type, const cooked_value_type &value)
{
    return value;
}

template< typename iterator, unsigned int size>
iterator write( rtlogs::signed_<size, rtlogs::big_endian>, long value, iterator destination)
{
    boost::endian::endian_store<long, size, boost::endian::order::big>( destination, value);
    return destination + size;
}

template< typename iterator, unsigned int size>
iterator write( rtlogs::unsigned_<size, rtlogs::big_endian>, unsigned long value, iterator destination)
{
    boost::endian::endian_store<unsigned long, size, boost::endian::order::big>( destination, value);
    return destination + size;
}

template< typename iterator, unsigned int size>
iterator write( rtlogs::signed_<size, rtlogs::little_endian>, long value, iterator destination)
{
    boost::endian::endian_store<long, size, boost::endian::order::little>( destination, value);
    return destination + size;
}

template< typename iterator, unsigned int size>
iterator write( rtlogs::unsigned_<size, rtlogs::little_endian>, unsigned long value, iterator destination)
{
    boost::endian::endian_store<unsigned long, size, boost::endian::order::little>( destination, value);
    return destination + size;
}

template< typename iterator>
iterator write( rtlogs::float32, float value, iterator destination)
{
    static_assert( sizeof( value) == rtlogs::float32::size, "This implementation expects the native float type to match IEEE 754");

    auto as_bytes = reinterpret_cast<unsigned char *>( &value);
    std::copy( as_bytes, as_bytes + rtlogs::float32::size, destination);

    return std::next( destination, rtlogs::float32::size);
}

template< typename iterator>
iterator write( rtlogs::nothing, long, iterator destination)
{
    return destination;
}



/**
 * for fixed point values, we can translate the floating point
 * value to a fixed point integer.
 */
template< typename int_type, unsigned int denom>
typename int_type::value_type to_raw_type( rtlogs::fixed_point< int_type, denom>, double value)
{
    return static_cast<typename int_type::value_type>( value * denom);
}

template< typename type_info, typename T>
T to_raw_type( const T& value)
{
    return value;
}

struct context
{
    int header;
    int channel;
    int column;
};

/**
 * pick a value from a vector of strings, convert it to some numerical value and write it to
 * a range of bytes.
 *
 * returns an iterator one past the last byte written.
 */
template< typename iterator, typename type>
iterator encode( type, const context &c, const std::vector<std::string> &values, iterator destination)
{
    const auto cooked = to_cooked_type<typename rtlogs::cooked_value_type<type>::type>( values[c.column]);
    const auto raw = to_raw_type( type{}, cooked);
    return write( typename type::raw_type_info{}, raw, destination);
}

template< typename iterator>
iterator encode( rtlogs::nothing, const context &c, const std::vector<std::string> &values, iterator destination)
{
    return destination;
}

template< typename iterator, unsigned int size>
iterator encode( rtlogs::ignore<size>, const context &c, const std::vector<std::string> &values, iterator destination)
{
    using std::next;
    std::fill( destination, next( destination, size), 0);
    return next(destination, size);
}


/**
 * Write the channel id to the range of bytes pointed to by the iterator 'destination'.
 */
template< typename iterator>
iterator encode( rtlogs::channel_id, const context &c, const std::vector<std::string> &, iterator destination)
{
    *destination = static_cast<unsigned char>( c.channel);
    return std::next( destination);
}

#endif /* PARSE_LOG_VALUE_ENCODING_HPP_ */
