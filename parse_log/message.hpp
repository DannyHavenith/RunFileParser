/*
 * message.hpp
 *
 *  Created on: Sep 17, 2011
 *      Author: danny
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <boost/mpl/vector.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

#include <boost/type_traits/is_same.hpp>

namespace rtlogs
{
    namespace mpl = boost::mpl;

    template< int code, int full_size>
    struct message
    {
        static const size_t size = full_size;
        static const size_t payload_size = (full_size == -1)?-1:full_size - 2;
        static const int header = code;
    };

    /// the symbol 'var' is used in the size part of messages to signify a variable size message.
    static const int var = -1;

    template< int code_begin, int code_end, int full_size>
    struct message_range
    {
        static const size_t size = full_size;
        static const size_t payload_size = (full_size == -1)?-1:full_size - 2;
        static const int header_begin = code_begin;
        static const int header_end = code_end;

        typedef void i_am_a_message_range;
    };

    /// meta-function is_message_range
    template< typename T, typename enable = void>
    struct is_message_range : mpl::false_ {};

    template< typename T>
    struct is_message_range< T, typename T::i_am_a_message_range> : mpl::true_ {};

    struct nothing
    {
        static const int size = 0;
    };

    template< int bytes>
    struct unsigned_
    {
        static const size_t size = bytes;
        typedef unsigned long value_type;
    };

    template< typename T>
    struct size
    {
        static const size_t value = T::size;
    };

    /// a message with fully specified payload types.
    template <int header, typename type1 = nothing, typename type2 = nothing, typename type3 = nothing>
    struct detailed_message : public message< header, size<type1>::value + size<type2>::value + size<type3>::value + 2>
    {
        typedef typename
                mpl::if_<
                    boost::is_same< type1, nothing>,
                    mpl::vector<>,
                    typename mpl::if_<
                        boost::is_same< type2, nothing>,
                        mpl::vector<type1>,
                        typename mpl::if_<
                            boost::is_same< type3, nothing>,
                            mpl::vector<type1, type2>,
                            mpl::vector<type1, type2, type3>
                        >::type
                    >::type
                >::type  parts;
    };

}


#endif /* MESSAGE_HPP_ */
