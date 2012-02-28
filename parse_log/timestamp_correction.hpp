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

// boost meta state machine includes
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/ref.hpp>

#include "messages.hpp"
#include "bytes_to_numbers.hpp"
#include "logscanner.hpp"

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
class slope_correction  : public rtlogs::messages_definition
{
public:
    slope_correction( std::ostream &output)
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
        if (lower <= upper)
        {
            lower_treshold = lower;
            upper_treshold = upper;
        }
        else
        {
            lower = 0;
            upper =  std::numeric_limits<timestamp_type>::max();
        }
    }

    ///
    /// most messages just get written to output
    template<typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        std::copy( begin, end, std::ostreambuf_iterator<char>( output));
    }

    /// time stamps get adapted before being sent to output
    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
        timestamp_type value = bytes_to_numbers::get_big_endian<3, unsigned long>( ++begin);

        // only allow time stamp values that are within reasonable limits. ignore all other time stamps.
        if (value >= lower_treshold && value <= upper_treshold)
        {

            // this line could be faster by pre-calculating offset = corrected_pivot - pivot * skew
            // and saying here: value = value * skew + offset
            // ... but it would be harder to see what's going on...
            value = (value - pivot) * skew + corrected_pivot;

            // create a timestamp message.
            unsigned char message[5] = { timestamp::header, value >> 16, value >> 8, value };
            message[4] = message[0] + message[1] + message[2] + message[3];

            std::copy( message, message + 5, std::ostreambuf_iterator<char>( output));
        }
    }


private:
    std::ostream    &output;
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
class time_correction_ :  public boost::msm::front::state_machine_def<time_correction_>
{
public:
    time_correction_( std::ostream &output)
    : correction( output)
    {

    }

    /**
     * Flush all buffered data after reprogramming the time stamp correction based on the given
     * time stamp and gps time values.
     */
    void flush(unsigned long  timestamp, unsigned long  gps_time)
    {
        unsigned long corrected_timestamp = ((previous_gps_time - first_gps_time) / gps_timestamp_ratio) + first_timestamp;
        double skew = (static_cast<double>( gps_time - previous_gps_time)/ gps_timestamp_ratio) / (timestamp - previous_timestamp);

        // set up the slope corrector and have it filter the contents of our buffer.
        correction.set_skew(previous_timestamp, corrected_timestamp, skew);
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
        first_timestamp = previous_timestamp = timestamp;
        first_gps_time = previous_gps_time = gps_time;
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
            fsm.store_first_timestamp(last_timestamp, last_gps_time);
        }
    };

    struct searching_ : public wedge_finder_
    {
        template< typename FSM>
        void on_exit(const time_ev & t, FSM &fsm)
        {
            fsm.flush( last_timestamp, last_gps_time);
        }

    };
    typedef boost::msm::back::state_machine<initial_ > initial;
    typedef boost::msm::back::state_machine<searching_ > searching;


    typedef initial initial_state;
    struct transition_table :
        boost::mpl::vector<
        //       Start                                Event             Next
        //   +--------------------------------------+-----------------+-----------+
        _row<  initial::exit_pt<initial::exit>      , time_ev         , searching> ,
        _row<searching::exit_pt<searching::exit>    , time_ev         , searching>
        >
    {
    };

private:
    typedef std::vector<unsigned char> buffer_type;
    buffer_type         buffer;
    slope_correction    correction;
    unsigned long first_timestamp;
    unsigned long first_gps_time;
    unsigned long previous_timestamp;
    unsigned long previous_gps_time;

    // ten times as many gps time ticks as timer ticks ( gps ticks are 1000/s while timer ticks are 100/s).
    const static unsigned long gps_timestamp_ratio = 10;
};

class time_correction : public boost::msm::back::state_machine<time_correction_>, public rtlogs::messages_definition
{
public:
    /// forward the reference parameter to the backend.
    /// we need create this constructor because the backend normally forwards all its constructor arguments
    /// by value and we don't want to burden clients with the task of remembering to wrap the stream in a ref(...) wrapper.
    time_correction( std::ostream &output)
    :boost::msm::back::state_machine<time_correction_>( boost::ref( output))
    {
        start();
    }

    ~time_correction()
    {
        final_flush();
    }

    /// most messages just get copied into the buffer
    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        add_to_buffer( begin, end);
    }

    template<typename iterator>
    void handle( rtlogs::parse_error, iterator, iterator)
    {
        // ignore all unparseable bytes.
    }

    template< typename iterator>
    void handle( timestamp, iterator begin, iterator end)
    {
       add_to_buffer( begin, end);
       process_event(
                time_ev( bytes_to_numbers::get_big_endian<3, unsigned long>(++begin))
                );
    }

    template< typename iterator>
    void handle( gps_time_storage, iterator begin, iterator end)
    {
        add_to_buffer( begin, end);
        process_event(
                gps_ev( bytes_to_numbers::get_big_endian<4, unsigned long>(++begin))
        );
    }

};

}


#endif /* TIMESTAMP_CORRECTION_HPP_ */
