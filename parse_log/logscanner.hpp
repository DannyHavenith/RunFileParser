/*
 * logscanner.hpp
 *
 *  Created on: Sep 18, 2011
 *      Author: danny
 */

#ifndef LOGSCANNER_HPP_
#define LOGSCANNER_HPP_
#include "parse_error.hpp"

namespace rtlogs
{
    /**
     * This type is used to scan through the bytes of a log. The log bytes to search through are provided
     * through iterators of type 'iterator'. Whenever a valid message is detected, this logscanner will call
     * an overload of the 'handle' member of a reactor. It will do this with the following arguments:
     * handle( m, b, e), where b and e are begin- and end iterators over the bytes of the message and m is an object
     * of the recognized message type (as declared in messages.h). The message object is only a tag type, used to dispatch to the
     * right overload of the handle function.
     *
     * the type 'iterator' must be a forward iterator, e.g. the iterator will dereferenced for reading, will be increased and copies will
     * be made.
     *
     */
    template< typename iterator, typename reactor_type>
    struct logscanner
    {
        /**
         * prepare a logscanner by giving it a reference to a reactor of which the 'handle' member function will be called.
         */
        logscanner( reactor_type &reactor)
        :reactor( reactor)
        {
            fill_handler_table();
        }

        /*
         * scan the log bytes in the iterator range [begin, end) and call the appropriate handler when messages are recognized.
         * When at some address 'a' no valid message is recognized, the scanner will try to continue at addres 'a+1' and repeat that until
         * a valid message is found.
         */
        void scan( iterator begin, iterator end)
        {
            while (begin != end)
            {
                // this depends on the handler to increase the begin-iterator if a message was recognized and to _not_ change the begin iterator
                // in case of failure.

                if (!table[*begin]->handle( reactor, begin, end))
                {
                    iterator start_of_garbage = begin;
                    ++begin;
                    while ( begin != end && !table[*begin]->handle( reactor, begin, end))
                    {
                        ++begin;
                    }

                    // tell the handler where the unreadable bytes are.
                    reactor.handle(  parse_error(), start_of_garbage, begin);
                }
            }
        }


    private:

        /**
         * Since for some messages the size is obtained from the message itself (the size is not fully determined by the first header byte),
         * we generalize this into a size_getter, that will return the size of a message, given a range of buffer bytes.
         *
         * Note that size is also a template argument. If this argument is anything other than -1, the get_size member must ignore the contents of
         * the buffer and return that arguments value. If it is -1, the size will be determined by reading the first byte after the header byte.
         */
        template<int size, typename dummy = void>
        struct size_getter
        {
            static int get_size( iterator, iterator)
            {
                return size;
            }
        };

        /**
         * Specialization of this template for size=-1, which actually gets the size of the message from the buffer itself.
         */
        template<typename dummy2>
        struct size_getter<-1, dummy2>
        {
            static int get_size( iterator first, iterator end)
            {
                iterator second = ++first;
                if (second != end)
                {
                    return *second + 3;
                }
                else
                {
                    return 1;
                }
            }
        };

        /**
         * a checker_impl instantiation will calculate the checksum of a message in the buffer and verify it.
         * The template parameter 'size' is the expected size of the message, including header- and checksum byte, or
         * -1 for variable-size messages.
         */
        template<int size>
        struct checker_impl
        {

            int get_size( iterator begin, iterator end)
            {
                return size_getter<size>::get_size( begin, end);
            }

            bool do_check( iterator &i, iterator end)
            {
                iterator save = i;
                unsigned char sum = 0;
                for (int count = get_size(i, end) - 1; count && i != end; --count)
                {
                    unsigned char value = *i++;
                    sum += value;
                }

                if (i == end || sum != *i++)
                {
                    i = save;
                    return false;
                }

                return true;
            }

        };

        /**
         * simple class that, given a template argument 'message' of some message tag type, will call
         * the appropriate 'handle' overload on a reactor.
         *
         * The reason for having this class, instead of directly calling the handle member function is that
         * this allows specializations on the message tag type.
         */
        template< typename message, typename enable = void>
        struct caller
        {
            static void call( reactor_type &r, iterator begin, iterator end)
            {
                r.handle( message(), begin, end);
            }
        };

        /**
         * This is the interface that all message handlers expose. See the handle member function.
         */
        struct handler_interface
        {
            /**
             * The handle function receives an iterator range and will try to determine whether the start of that buffer points to a valid message.
             * If the message is valid (determined by its checksum), an appropriate member function of the given reactor will be called (the 'handle(m, begin, end)'-function) and
             * the begin itertator is advanced to a postion just after the last byte of the message.
             * If the checskum fails, th function must return false and not change the given begin-iterator.
             */
            virtual bool handle( reactor_type &r, iterator &i, iterator end) = 0;
        };

        /**
         * The generic handler for not-implemented messages (header bytes values that do not have an associated message).
         */
        struct null_handler : public handler_interface
        {
            /**
             * singleton implemenation (remind me to do the private constructor stuff as well).
             */
            static null_handler &instance()
            {
                static null_handler h;
                return h;
            }

            /**
             * for unknown header values, we just return false.
             */
            virtual bool handle( reactor_type &r, iterator &i, iterator end)
            {
                return false;
            }
        };

        /**
         * Generic handler that will be instantiated for each known message type.
         * This handler will first do a checksum test and if that succeeds will call the 'handle' member function on
         * a provided reactor.
         */
        template <typename message>
        struct handler : public handler_interface, private checker_impl< size< message>::value>
        {
            // singleton.
            static handler &instance()
            {
                static handler h;
                return h;
            }

            /**
             * This function performs a checksum test and if that succeeds will call the 'handle' member function
             * of the provided reactor.
             */
            virtual bool handle( reactor_type &r, iterator &i, iterator end)
            {
                iterator begin = i;

                // do_check will increase i to be one-beyond the end of the message
                // if the check succeeds.
                if (!do_check(i, end)) return false;

                caller<message>::call( r, begin, i);

                return true;
            }
        };

        /// We'll be creating a pointer-to-handler for each possible byte value.
        typedef handler_interface *table_type[256];

        /**
         * This class is used in conjunction with a boost.fusion for_each loop to
         * fill a table with pointers to appropriate handlers for each message header value.
         */
        struct cell_filler
        {
            cell_filler( table_type &table)
            :table(table)
            {

            }

            /**
             * Fill a single entry in the handler table with a handler for the given message type.
             */
            template< typename T>
            typename boost::disable_if< is_message_range<T> >::type
            operator()( const T & ) const
            {
                table[T::header] = &handler<T>::instance();
            }

            /**
             * Fill several entries in the handler table for 'message ranges'.
             */
            template< typename T>
            typename boost::enable_if< is_message_range<T> >::type
            operator()( const T & ) const
            {
                for ( int i = T::header_begin; i != T::header_end; ++i)
                {
                    table[i] = &handler<T>::instance();
                }
            }

        private:
            /// table with a handler for each possible byte value.
            table_type &table;
        };

        /**
         * Fill the handler table with pointers to handlers for each known message type. Fill the other cells of the table
         * with pointers to the null handler.
         */
        void fill_handler_table( )
        {
            std::fill( table, table + 256, &null_handler::instance());
            boost::fusion::for_each( commands(), cell_filler(table));
        }

        table_type      table;
        reactor_type    &reactor;
    };

    /**
     * scan the log bytes provided through iterators begin and end, decode messages and call the approprate overload of
     * the 'handle' member function on the provided reactor type object.
     */
    template<typename iterator, typename reactor>
    void scan_log( reactor &r, iterator begin, iterator end)
    {
        logscanner<iterator, reactor> scanner( r);
        scanner.scan( begin, end);
    }
}


#endif /* LOGSCANNER_HPP_ */
