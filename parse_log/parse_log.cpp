// parse_log.cpp : Defines the entry point for the console application.
//

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/mpl/size.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>

#include "messages.hpp"
#include "logscanner.hpp"

using namespace rtlogs;

struct clean_file_writer
{
    clean_file_writer( const boost::filesystem::path &base)
    :parent_path( base.parent_path()), base(basename(base)), extension( boost::filesystem::extension( base)),
     last_timestamp(0)
    {
        file_suffix[0] = 'a';
        file_suffix[1] = 0;

        open_next_file();
    }

    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        // write the packet to the current output file.
        std::copy( begin, end, ostreambuf_iterator( output));
    }

    /**
     * handle timer messages. When the timestamp makes a large jump, or becomes lower than the previous value,
     * a new file is opened and output will be delegated to that file.
     */
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        iterator current = begin;
        ++current;
        unsigned long time_value = *current++;
        time_value = (time_value << 8) + * current++;
        time_value = (time_value << 8) + * current++;

        if (last_timestamp && (last_timestamp > time_value || time_value - last_timestamp > 5000))
        {
            open_next_file();
        }

        // write the packet to the current output file.
        std::copy( begin, end, ostreambuf_iterator( output));

        last_timestamp = time_value;
    }

private:

    /**
     * open a new output file. Files will be typically named "<inputfilebase>'a'.<inputfileext>", "<inputfilebase>'b'.<inputfileext>", etc.
     */
    void open_next_file()
    {
        if (output.is_open())
        {
            output.close();
        }
        std::cerr << parent_path / (base + file_suffix + extension)  << '\n';
        output.open(  parent_path / (base + file_suffix + extension) , std::ios::binary);
        if (!output.is_open())
        {
            throw std::runtime_error("could not open output file");
        }
        ++file_suffix[0];
    }

    typedef std::ostreambuf_iterator<char> ostreambuf_iterator;

    const boost::filesystem::path parent_path;
    const std::string           base;
    char                        file_suffix[2];
    const std::string           extension;
    boost::filesystem::ofstream output;
    unsigned long               last_timestamp;
};
/**
 * This class handles timestamp messages only. It prints the timestamp for every unique timestamp it receives, together
 * with a count of how often that same value was encountered.
 */
struct timestamp_printer
{
    timestamp_printer( std::ostream &out)
        :out( out), last_timestamp(0), timestamp_count(0),  first_timestamp(0){};

    /// do nothing with most messages.
    void handle( ...)
    {
    };

    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        ++begin;
        unsigned long result = *begin++;
        result = (result << 8) + * begin++;
        result = (result << 8) + * begin++;
        if (result == last_timestamp)
        {
            ++timestamp_count;
        }
        else
        {
            if (first_timestamp == 0)
            {
                first_timestamp = result;
            }

            out << '\t' << timestamp_count << '\t' << result - last_timestamp << '\n';
            last_timestamp = result;
            timestamp_count = 1;
            out << result;
        }
    }

    void flush()
    {
        out << '\t' << timestamp_count << '\n';
        out << "time span: " << last_timestamp - first_timestamp << '\n';
    }

private:
    std::ostream &out;

    unsigned long last_timestamp;
    unsigned long timestamp_count;
    unsigned long first_timestamp;
};

struct text_printer
{
    text_printer( std::ostream &out)
    :out(out) {}

    template< typename message_type, typename iterator>
    void handle( message_type, iterator begin, iterator end)
    {
        out << message_type::description();
        while (begin != end)
        {
            out << '\t' << (int)*begin;
            ++begin;
        }
        out << '\n';
    }

private:
    std::ostream &out;
};

struct kml_writer
{
    kml_writer( std::ostream &out)
       :out(out)
    {
        out << prolog();
        out.precision(8);
    }

    void handle(...){};

    template< typename iterator>
    void handle( gps_position, iterator begin, iterator end)
    {
        ++begin;

        typedef boost::int32_t long_type;
        long_type longitude = *begin++;
        longitude = (longitude << 8) + *begin++;
        longitude = (longitude << 8) + *begin++;
        longitude = (longitude << 8) + *begin++;
        double longitude_double = longitude/10000000.0;

        long_type lattitude = *begin++;
        lattitude = (lattitude << 8) + *begin++;
        lattitude = (lattitude << 8) + *begin++;
        lattitude = (lattitude << 8) + *begin++;
        double lattitude_double = lattitude/10000000.0;

        out <<  "        " << longitude_double << ',' << lattitude_double << ",0.0\n";
    }

    ~kml_writer()
    {
        out << epilog();
    }

private:
    static const char *prolog()
    {
        return
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<kml xmlns=\"http://earth.google.com/kml/2.2\">\n"
            "<Placemark>\n"
            "    <name>Path255</name>\n"
            "    <Style>\n"
            "        <LineStyle>\n"
            "            <color>ff0000ff</color>\n"
            "            <width>3.1</width>\n"
            "        </LineStyle>\n"
            "    </Style>\n"
            "    <LineString>\n"
            "        <tessellate>1</tessellate>\n"
            "        <coordinates>\n"
            ;
    }

    static const char *epilog()
    {
        return
            "        </coordinates>\n"
            "    </LineString>\n"
            "</Placemark>\n"
            "</kml>\n"
            ;
    }

    std::ostream &out;
};

void error( const std::string &what)
{
    throw std::runtime_error( what);
}

int main(int argc, char* argv[])
{
    using namespace std;
    typedef std::vector<unsigned char> buffer_type;

    try
    {
        if (argc < 2)
        {
            error("specify a file name");
        }

        // open the log file
        ifstream file( argv[1], ios_base::binary);
        if (!file)
        {
            error( string("could not open file: ") + argv[1]);
        }

        // copy the file contents into a buffer
        file.unsetf( ios_base::skipws);
        istreambuf_iterator<char> begin( file), end;
        const buffer_type buffer( begin, end);

        if (argc == 3)
        {
            if (argv[2] == string("timestamps"))
            {
                // now scan the log bytes in the buffer.
                timestamp_printer printer(cout);
                scan_log( printer, buffer.begin(), buffer.end());
                // cleanup
                printer.flush();
            }
            else if (argv[2] == string("txt"))
            {
                text_printer printer( cout);
                scan_log( printer, buffer.begin(), buffer.end());
            }
            else if (argv[2] == string("clean"))
            {
                clean_file_writer writer( argv[1]);
                scan_log( writer, buffer.begin(), buffer.end());
            }
            else
            {
                std::cerr << "could not interpret command: " << argv[2] << "\n";
            }
        }
        else
        {
            kml_writer kml( cout);
            scan_log( kml, buffer.begin(), buffer.end());
        }

    }
    catch (exception &e)
    {
        cerr << "something went wrong: " << e.what() << '\n';
    }

	return 0;
}

