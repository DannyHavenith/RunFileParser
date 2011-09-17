// parse_log.cpp : Defines the entry point for the console application.
//

#include "messages.hpp"

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <vector>

typedef std::vector<unsigned char> buffer_type;


template<typename iterator, int size>
struct size_getter
{
    static int get_size( iterator, iterator)
    {
        return size;
    }
};

template< typename iterator>
struct size_getter< iterator, -1>
{
    static int get_size( iterator first, iterator end)
    {
        iterator second = ++first;
        if (second != end)
        {
            return *second;
        }
        else
        {
            return 1;
        }
    }
};

template<typename iterator>
struct checker_interface
{
    virtual bool check( iterator &i, iterator end) = 0;
};

template<typename iterator, int size>
struct checker : public checker_interface<iterator>
{
    static checker<iterator, size> &instance()
    {
        static checker<iterator, size> the_instance;
        return the_instance;
    }

    int get_size( iterator begin, iterator end)
    {
        return size_getter<iterator, size>::get_size( begin, end);
    }

    virtual bool check( iterator &i, iterator end)
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

typedef checker_interface<buffer_type::const_iterator> *table_type[256];
struct cell_filler
{
    cell_filler( table_type &table)
    :table(table)
    {

    }

    template< int header, int size>
    void operator()( const message<header, size> & ) const
    {
        std::cout << header << ":";
        table[header] = &checker< buffer_type::const_iterator, size>::instance();
    }

    template< int header_begin, int header_end, int size>
    void operator()( const message_range< header_begin, header_end, size>& ) const
    {
        for ( int i = header_begin; i != header_end; ++i)
        {
            table[i] = &checker< buffer_type::const_iterator, size>::instance();
        }
    }

private:
    table_type &table;
};

void fill_checker_table( table_type &table)
{
    std::fill( table, table + 256, static_cast<checker_interface<buffer_type::const_iterator> *>( 0));
    boost::fusion::for_each( commands(), cell_filler(table));
}

void error( const std::string &what)
{
    throw std::runtime_error( what);
}

// global, so zero-initialized
size_t good_count[256];
size_t bad_count[256];

void scan_events( buffer_type::const_iterator begin, buffer_type::const_iterator end)
{
    using namespace std;

    table_type table;
    fill_checker_table( table);

    unsigned char firstbyte = 0;
    while (begin != end)
    {

        firstbyte = *begin;
        while (begin != end && table[*begin] && table[*begin]->check( begin, end))
        {
            ++good_count[firstbyte];
            firstbyte = *begin;
            cout << '.';
        }

        if (begin != end)
        {
            size_t skipcount = 0;
            firstbyte = *begin;
            while ( begin != end &&  (!table[*begin] ||  !table[*begin]->check( begin, end)))
            {
                ++begin;
                ++skipcount;
            }
            ++bad_count[firstbyte];
            cout << "\nskipped " << skipcount << " bytes, starting with " << (int)firstbyte << "\n";
        }
    }
}

int main(int argc, char* argv[])
{
    using namespace std;

    try
    {
        if (argc < 2)
        {
            error("specify a file name");
        }

        ifstream file( argv[1], ios_base::binary);
        if (!file)
        {
            error( string("could not open file: ") + argv[1]);
        }

        file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( file), end;
        const vector<unsigned char> buffer( begin, end);

        cout << "buffer is " << buffer.size() << " bytes large\n";

        scan_events( buffer.begin(), buffer.end());

        for (int i = 0; i < 256; ++i)
        {
            cout << i << "    " << good_count[i] << "    " << bad_count[i] << '\n';
        }

        cout << "finished\n";
    }
    catch (exception &e)
    {
        cerr << "something went wrong: " << e.what() << '\n';
    }

	return 0;
}

