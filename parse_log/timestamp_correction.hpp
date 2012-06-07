/*
 * timestamp_correction.hpp
 *
 *  Created on: Feb 17, 2012
 *      Author: danny
 */

#ifndef TIMESTAMP_CORRECTION_HPP_
#define TIMESTAMP_CORRECTION_HPP_
#include <vector>
#include <iterator>
#include <limits>
#include <iostream>
#include <iterator>
#include <algorithm>

// boost meta state machine includes
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/ref.hpp>

#include "messages.hpp"
#include "bytes_to_numbers.hpp"
#include "logscanner.hpp"

/**
 * A simple handler class that will just write all parsed messages to a given output file.
 */
class binary_file_writer
{
public:
    binary_file_writer( std::ostream &output)
    :output( output)
    {
        // write a header to the file
       static const char header[] = {0x98 , 0x1d , 0x00 , 0x00 , 0xc8 , 0x00 , 0x00 , 0x00};
       std::copy( header, header + sizeof header, std::ostreambuf_iterator<char>( output));
    }

    /**
     * all messages are just written verbatim to the output file.
     */
    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        std::copy( begin, end, std::ostreambuf_iterator<char>( output));
    }

private:
    std::ostream &output;
};


namespace timestamp_correction
{

/**
 * Class that adapts time stamp messages by changing their 'slope' and pivot point.
 *
 * This class writes all incoming packages to a standard output stream. Time stamps are
 * interpreted and their time stamp value may be adjusted by setting a pivot, a 'corrected pivot'
 * and a skew value. Strictly speaking only one pivot is necessary, but specifying two pivots
 * makes the definition more intuitive.
 *
 * The idea is that time stamp values describe a straight line that passes through a pivot point.
 * This class will multiply the values with a skew rate to change the slope and 'shift' the
 * pivot point up or down to a new 'corrected' pivot.
 *
 * <pre>
 *                   ++  corrected slope
 *                 ++
 *               ++
 *             ++
 *   cpivot->*+            xxxx  original slope
 *         ++^         xxxx
 *       ++  |     xxxx
 *     ++    | xxxx
 *         xx*x
 *           ^
 *           pivot
 * </pre>
 * This class can only perform one linear correction on a range of time stamp values. It can only be used
 * with the same settings in regions where relation between the old and corrected time stamps is linear.
 * The time_stamp_correcter class cuts the data streams into segments where this relation is indeed linear and
 * programs an object of this slope correction class accordingly.
 */
template< typename output_handler>
class slope_correction  : public rtlogs::messages_definition
{
public:
    slope_correction( output_handler &output)
    :output( output), pivot(0), corrected_pivot(0), skew(1.0),
     lower_treshold(0), upper_treshold( std::numeric_limits<timestamp_type>::max()){}

    typedef unsigned long timestamp_type;

    /// set the time stamp correction parameters.
    void set_skew( timestamp_type new_pivot, timestamp_type new_corrected_pivot, double new_skew)
    {
        pivot           = new_pivot;
        corrected_pivot = new_corrected_pivot;
        skew            = new_skew;
    }

    /// configure a time stamp filter that allows only time stamps that are within the given range.
    /// The allowed range includes both the lower and the upper limit.
    /// When no upper range is provided, only the lower range will be used to filter
    void set_allowed_range( timestamp_type lower, timestamp_type upper = std::numeric_limits<timestamp_type>::max())
    {
        lower_treshold = lower;
        upper_treshold = upper;
    }

    ///
    /// most messages just get send to output
    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        output.handle( message(), begin, end);
        //std::copy( begin, end, std::ostreambuf_iterator<char>( output));
    }

    /// time stamps get adapted before being sent to output
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        timestamp_type value = bytes_to_numbers::get_big_endian<3, unsigned long>( ++begin);

        // only allow time stamp values that are within reasonable limits. ignore all other time stamps.
        // note: I'm counting on unsigned integer underflow here. The reason for specifying the condition this way
        // is that now I can now give a range like [max - 10, 10] and this will accept values from max-10 up to max and values
        // from 0 to 10, with max being the maximum (24-bit) time stamp value.
        if (value - lower_treshold <= upper_treshold)
        {

            // this line could be faster by pre-calculating offset = corrected_pivot - pivot * skew
            // and saying here: value = value * skew + offset
            // ... but it would be harder to see what's going on...
            value = (value - pivot) * skew + corrected_pivot;

            // create a timestamp message.
            unsigned char message[5] = { timestamp::header, value >> 16, value >> 8, value };
            message[4] = message[0] + message[1] + message[2] + message[3];
            output.handle( timestamp(), message, message + 5);
            //std::copy( message, message + 5, std::ostreambuf_iterator<char>( output));
        }
    }


private:
    output_handler  &output;
    timestamp_type  pivot;
    timestamp_type  corrected_pivot;
    double          skew;
    timestamp_type  lower_treshold;
    timestamp_type  upper_treshold;
};

/**
 * Boost.msm frontend class for the wedge_finder state machine.
 *
 * This class receives GPS (G) and TS (T) events (GPS time channel or Time Stamp). It tries to find
 * a succession T G T, where the time stamp value of the two T events are very close. This means that
 * the value of the G event can be correlated with the value of the first T event.
 *
 * The state machine implemented by this class looks like this:
 *
 * <pre>
 *     +-gps-+             +-time-+
 *     v     |             v      |
 *  (searching) --time--> (ts_found) --------gps--> (gps_found) --time[timestamps_close]--> (exit)
 *      ^                   ^                          |   |
 *      |                   +-time[~timestamps_close]--+   |
 *      +-----------------------------------gps----------- +
 * </pre>
 */
struct wedge_finder_ : public boost::msm::front::state_machine_def<wedge_finder_>
{

    //**********
    // events
    //**********

    /// gps event
    struct gps_ev
    {
        explicit gps_ev( unsigned long value) : value( value) {}
        unsigned long value;
    };

    /// timestamp event
    struct time_ev
    {
        explicit time_ev( unsigned long value) : value( value){}
        unsigned long value;
    };

    //**********
    // states
    //**********
    struct searching :  boost::msm::front::state<>
    {
    };
    struct ts_found  :  boost::msm::front::state<>
    {

        template< typename Event, typename FSM>
        void on_entry( const Event&, const FSM &) {}

        template<typename FSM>
        void on_entry( const time_ev &time, FSM &fsm)
        {
            fsm.last_timestamp = time.value;
        }
    };
    // this is an exit pseudo state that emits a 'time' event while exiting.
    struct exit : boost::msm::front::exit_pseudo_state<time_ev>
    {
    };
    struct gps_found :  boost::msm::front::state<>
    {

        template< typename Event, typename FSM>
        void on_entry( const Event&, const FSM &) {}

        template<typename FSM>
        void on_entry( const gps_ev &gps_time, FSM &fsm )
        {
            fsm.last_gps_time = gps_time.value;
        }
    };

    typedef searching initial_state;

    /// is the given time events value close to that of the previously offered time event?
    bool timestamps_close( const time_ev &time_event)
    {
        return time_event.value > last_timestamp && time_event.value - last_timestamp < close_time_treshold;
    }

    //**********
    // state transition table
    //**********
    typedef wedge_finder_ wf;
    struct transition_table : boost::mpl::vector<
    //       Start      Event           Next       Action               Guard
    //   +-----------+-----------------+-----------+---------------------+----------------------+
     _row< searching , time_ev         , ts_found                                               >,
     _row< searching , gps_ev          , searching                                              >,
     _row< ts_found  , time_ev         , ts_found                                               >,
     _row< ts_found  , gps_ev          , gps_found                                              >,
     _row< gps_found , gps_ev          , searching                                              >,
     _row< gps_found , time_ev         , ts_found                                               >,
    g_row< gps_found , time_ev         , exit      ,                       &wf::timestamps_close>
    >{};


    // time stamps are close if their values are less than this threshold apart
    const static unsigned long  close_time_treshold = 10;
    unsigned long           last_timestamp;
    unsigned long           last_gps_time;
};




/**
 * The time correction state machine.
 *
 * This class describes a state machine that consists of two states, each sub-state machines of type wedge_finder.
 * The first state is the initial state, which is active as long as no TGT-wedge has been found (@see wedge_finder_).
 *
 * After the first wedge has been found, this machine remains in the searching state, returning to the searching
 * state with each wedge found. Whenever a wedge is found, the embedded slop correction object is configured and all message are flushed to
 * its output stream.
 *
 */
template< typename output_handler>
class time_correction_ :  public boost::msm::front::state_machine_def<time_correction_<output_handler> >
{
public:
    typedef boost::msm::front::state_machine_def<time_correction_<output_handler> > front_end;
    time_correction_( output_handler &output)
    : correction( output), first_gps_time(0), previous_gps_time(0), previous_timestamp(0), skew(1.0)
    {

    }

    /**
     * Flush all buffered data after reprogramming the time stamp correction based on the given
     * time stamp and gps time values.
     */
    void flush(unsigned long  timestamp, unsigned long  gps_time)
    {

        if (timestamp > previous_timestamp)
        {
            // calculate the value that we think the time stamp should have, given the progression of gps time stamps.
            // note that this value could be greater than 2^24, the maximum time stamp, but that is OK, we can just let the time stamps roll
            // over to zero modulo  2^24
            unsigned long corrected_timestamp = ((previous_gps_time - first_gps_time) / gps_timestamp_ratio) + first_timestamp;
            skew = (static_cast<double>( gps_time - previous_gps_time)/ gps_timestamp_ratio) / (timestamp - previous_timestamp);

            // set up the slope corrector and have it filter the contents of our buffer.
            correction.set_skew(previous_timestamp, corrected_timestamp, skew);
        }

        // due to the way the range is treated, this will also work when timestamp is smaller than previous_timestamp by overflow.
        // it will then simply allow the range [previous_timestamp, max_timestamp] and [0, timestamp].
        correction.set_allowed_range( previous_timestamp, timestamp);
        flush();

        previous_timestamp = timestamp;
        previous_gps_time  = gps_time;
    }


    /**
     * Flush all bytes in the buffer, using the last known good settings for time correction
     */
    void final_flush()
    {
        correction.set_allowed_range( 0);
        flush();
    }

    /**
     * Send all bytes in the buffer to the time stamp correction class.
     */
    void flush()
    {
        rtlogs::scan_log(correction, buffer.begin(), buffer.end());
        buffer.clear();
    }


    /**
     * This function is called when the first time stamp/gps time pair is found.
     */
    void store_first_timestamp(unsigned long  timestamp, unsigned long  gps_time)
    {
        previous_timestamp = timestamp;
        first_gps_time = previous_gps_time = gps_time;
        skew = 1.0;
    }


    /// add a range of bytes to the buffer.
    template<typename iterator>
    void add_to_buffer( iterator begin, iterator end)
    {
        std::copy( begin, end, std::back_inserter( buffer));
    }


    typedef wedge_finder_::gps_ev gps_ev;
    typedef wedge_finder_::time_ev time_ev;
    struct initial_ : public wedge_finder_
    {

        template< typename FSM>
        void on_exit(const time_ev & t, FSM &fsm)
        {
            fsm.store_first_timestamp(t.value, last_gps_time);
        }
    };

    struct searching_ : public wedge_finder_
    {
        template< typename FSM>
        void on_exit(const time_ev & t, FSM &fsm)
        {
            fsm.flush( t.value, last_gps_time);
        }

    };
    typedef boost::msm::back::state_machine<initial_ > initial;
    typedef boost::msm::back::state_machine<searching_ > searching;
    //typedef typename front_end::template _row<  initial:: template exit_pt<typename initial::exit>      , time_ev         , searching> row1type;

    typedef initial initial_state;

    // I need quite a lot of 'template' and 'typename' boilerplate here because the compiler can't make any assumptions anymore since frontend is a
    // Dependent name. Need to figure out a way to make this simpler, this beats the purpose (readability) of a state transition table.
    struct transition_table :
        boost::mpl::vector<
        //                                  Start                                                                Event             Next
        //                                  +------------------------------------------------------------------+-----------------+-----------+
        typename front_end::template _row<  typename   initial:: template exit_pt<typename initial::exit>      , time_ev         , searching> ,
        typename front_end::template _row<  typename searching:: template exit_pt<typename searching::exit>    , time_ev         , searching>
        >
    {
    };

private:
    typedef std::vector<unsigned char> buffer_type;
    buffer_type         buffer;
    slope_correction<output_handler> correction;

    /// this is the value that we give the time stamp that is associated with the first gps time packet.
    /// it can't be zero, because there might be time stamps before the first gps time packet and those
    /// should get a lower value.
    static const unsigned long first_timestamp = 15000;

    unsigned long first_gps_time;
    unsigned long previous_timestamp;
    unsigned long previous_gps_time;
    double        skew;

    // ten times as many gps time ticks as timer ticks ( gps ticks are 1000/s while timer ticks are 100/s).
    const static unsigned long gps_timestamp_ratio = 10;
};

/**
 * This class is the front end to the time correction state machine.
 *
 *
 */
template< typename output_handler>
class time_correction : public boost::msm::back::state_machine<time_correction_<output_handler> >, public rtlogs::messages_definition
{
public:
    typedef boost::msm::back::state_machine<time_correction_<output_handler> > state_machine;

    /// forward the reference parameter to the backend.
    /// we need create this constructor because the backend normally forwards all its constructor arguments
    /// by value and we don't want to burden clients with the task of remembering to wrap the stream in a ref(...) wrapper.
    time_correction( output_handler &output)
    :boost::msm::back::state_machine<time_correction_<output_handler> >( boost::ref( output))
    {
        state_machine::start();
    }

    ~time_correction()
    {
        state_machine::final_flush();
    }

    /// most messages just get copied into the buffer
    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        add_to_buffer( begin, end);
    }

    /// messages that cannot be parsed are ignored.
    template<typename iterator>
    void handle( rtlogs::parse_error, iterator, iterator)
    {
        // ignore all unparseable bytes.
    }

    /**
     * timestamp messages generate events.
     */
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
       add_to_buffer( begin, end);
       state_machine::process_event(
       typename state_machine::time_ev( bytes_to_numbers::get_big_endian<3, unsigned long>(++begin))
                );
    }

    /**
     * gps time events generate events.
     */
    template< typename iterator>
    void handle( gps_time_storage, iterator begin, iterator end)
    {
        add_to_buffer( begin, end);
        process_event(
                typename state_machine::gps_ev( bytes_to_numbers::get_big_endian<4, unsigned long>(++begin))
        );
    }

};

}


#endif /* TIMESTAMP_CORRECTION_HPP_ */
