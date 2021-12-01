/*
 * message.hpp
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Sep 17, 2011
 *      Author: danny
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <boost/mpl/vector.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/type_traits/is_same.hpp>

namespace rtlogs
{
    namespace mpl = boost::mpl;

    template< int code, int full_size>
    struct message
    {
        static const int size = full_size;
        static const int payload_size = (full_size == -1)?-1:full_size - 2;
        static const int header = code;
    };

    /// the symbol 'var' is used in the size part of messages to signify a variable size message.
    constexpr int var = -1;

    template< int code_begin, int code_end, int full_size>
    struct message_range
    {
        static const int size = full_size;
        static const int payload_size = (full_size == -1)?-1:full_size - 2;
        static const int header_begin = code_begin;
        static const int header_end = code_end;

        typedef void i_am_a_message_range;
    };

    /// meta-function is_message_range
    template< typename T, typename enable = void>
    struct is_message_range : mpl::false_ {};

    /// if type T has an embedded type 'i_am_a_message_range', then specialization of the
    /// meta function will return true
    /// partially specializing on message_range<int, int, int> did not work for some reason on g++.
    template< typename T>
    struct is_message_range< T, typename T::i_am_a_message_range> : mpl::true_ {};

    ///
    /// place holder for unused members of detailed_message.
    struct nothing
    {
        static const int size = 0;
    };

    struct big_endian {};
    struct little_endian {};

    /**
     * This represents a channel id in the message, such as the one used for
     * Auxiliary channel messages.
     */
    struct channel_id
    {
        static constexpr int size = 1;
        using value_type = unsigned int;
        using cooked_value_type = value_type;
        using byte_order = little_endian;
        using raw_type_inf = channel_id;
    };

    /**
     * This represents an unsigned integer in the message.
     */
    template< unsigned int bytes, typename order = little_endian>
    struct unsigned_
    {
        static const int size = bytes;
        using value_type = unsigned long;
        using cooked_value_type = value_type;
        using byte_order = order;
        using raw_type_info = unsigned_<bytes, order>;
    };

    /**
     * This represents a signed integer in the message
     */
    template<unsigned int bytes, typename order = little_endian>
    struct signed_
    {
        static const int size = bytes;
        using value_type = long;
        using cooked_value_type = value_type;
        using byte_order = order;
        using raw_type_info = signed_<bytes, order>;
    };

    template< unsigned int bytes>
    struct ignore
    {
        static const int size = bytes;
        using value_type = void;
        using cooked_value_type = value_type;
        using raw_type_info = ignore<bytes>;
    };

    /**
     * This represents a value in
     * IEEE 754 single precision floating point format
     */
    struct float32
    {
        static const int size = 4;
        using value_type = float;
        using cooked_value_type = value_type;
        using raw_type_info = float32;
    };

    /**
     * This represents a fixed point type in the message.
     *
     * fixed point numbers are represented by some integer in the
     * message divided by some constant.
     */
    template< typename int_type, unsigned int denom>
    struct fixed_point : public int_type
    {
        using cooked_value_type = double;
        constexpr static int denominator = denom;
    };

    template< typename T>
    struct size
    {
        static const int value = T::size;
    };

    template< typename typeinfo>
    struct cooked_value_type
    {
        using type = typename typeinfo::cooked_value_type;
    };


    /// a message with fully specified payload types.
    template <int header, typename type1 = nothing, typename type2 = nothing, typename type3 = nothing>
    struct detailed_message : public message< header, size<type1>::value + size<type2>::value + size<type3>::value + 2>
    {
        using parts = mpl::vector< type1, type2, type3>;
    };

    template< int code_begin, int code_end, typename type1 = nothing, typename type2 = nothing, typename type3 = nothing>
    struct detailed_message_range : public message_range< code_begin, code_end, size<type1>::value + size<type2>::value + size<type3>::value + 2>
    {
        using parts = mpl::vector< type1, type2, type3>;
    };

}


#endif /* MESSAGE_HPP_ */
