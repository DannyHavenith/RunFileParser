/*
 * logscanner.hpp
 *
 *  Created on: Sep 18, 2011
 *      Author: danny
 */

#ifndef LOGSCANNER_HPP_
#define LOGSCANNER_HPP_

namespace rtlogs
{
    template< typename iterator, typename reactor_type>
    struct logscanner
    {
        logscanner( reactor_type &reactor)
        :reactor( reactor)
        {
            fill_handler_table();
        }

        void scan( iterator begin, iterator end)
        {
            while (begin != end)
            {
                if (!table[*begin]->handle( reactor, begin, end))
                {
                    ++begin;
                }
            }
        }

        struct checksum_error {};
        struct unknown_message {};


    private:

        template<int size, typename dummy = void>
        struct size_getter
        {
            static int get_size( iterator, iterator)
            {
                return size;
            }
        };

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

        template< typename message, typename enable = void>
        struct caller
        {
            static void call( reactor_type &r, iterator begin, iterator end)
            {
                r.handle( message(), begin, end);
            }
        };

        struct handler_interface
        {
            virtual bool handle( reactor_type &r, iterator &i, iterator end) = 0;
        };

        struct null_handler : public handler_interface
        {
            static null_handler &instance()
            {
                static null_handler h;
                return h;
            }

            virtual bool handle( reactor_type &r, iterator &i, iterator end)
            {
                return false;
            }
        };

        template <typename message>
        struct handler : public handler_interface, private checker_impl< size< message>::value>
        {
            static handler &instance()
            {
                static handler h;
                return h;
            }

            virtual bool handle( reactor_type &r, iterator &i, iterator end)
            {
                iterator begin = i;
                if (!do_check(i, end)) return false;
                caller<message>::call( r, begin, end);
                return true;
            }
        };

        typedef handler_interface *table_type[256];

        struct cell_filler
        {
            cell_filler( table_type &table)
            :table(table)
            {

            }

            template< typename T>
            typename boost::disable_if< is_message_range<T> >::type
            operator()( const T & ) const
            {
                table[T::header] = &handler<T>::instance();
            }

            template< int header_begin, int header_end, int size>
            void operator()( const message_range< header_begin, header_end, size>& ) const
            {
                for ( int i = header_begin; i != header_end; ++i)
                {
                    table[i] = &handler< message_range< header_begin, header_end, size> >::instance();
                }
            }

        private:
            table_type &table;
        };

        void fill_handler_table( )
        {
            std::fill( table, table + 256, &null_handler::instance());
            boost::fusion::for_each( commands(), cell_filler(table));
        }

        table_type      table;
        reactor_type    &reactor;
    };

    template<typename iterator, typename reactor>
    void scan_log( reactor &r, iterator begin, iterator end)
    {
        logscanner<iterator, reactor> scanner( r);
        scanner.scan( begin, end);
    }
}


#endif /* LOGSCANNER_HPP_ */
