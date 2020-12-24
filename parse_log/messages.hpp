/*
 * messages.hpp
 * Copyright (C) 2011, 2012 Danny Havenith
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 *  Created on: Sep 12, 2011
 *      Author: danny
 */

#ifndef MESSAGES_HPP_
#define MESSAGES_HPP_

#include <boost/mpl/vector.hpp>
#include <boost/mpl/joint_view.hpp>
#include "message.hpp"

namespace rtlogs
{
using boost::mpl::joint_view;

/// This struct type contains a huge list of struct typedefs where each struct represents a known rtlog message type.
/// All structs derive from the message<header, size> template and use that to encode information about each message.
/// Additionally, this struct contains an embedded typedef, called list, that is an mpl sequence of all the structs.
struct messages_definition
{
    struct run_information  : message<1, 9>     { static const char *description() { return  "Run Information";}};
    struct run_start_stop   : message<2, 11>    { static const char *description() { return  "Run start/stop info";}};
    struct raw_gps          : message<3, var>    { static const char *description() { return  "Raw GPS Data Input";}};
    struct new_sector_time  : message<4, 7>     { static const char *description() { return  "New Sector Time";}};
    struct new_lap_marker   : message<5, 21>    { static const char *description() { return  "New Lap Marker";}};
    struct logger_storage   : message<6, 6>     { static const char *description() { return  "Logger Storage Channel";}};
    struct gps_time_storage : message<7, 6>     { static const char *description() { return  "GPS Time Storage Channel";}};
    struct accelerations    : message<8, 6>     { static const char *description() { return  "Accelerations";}};
    struct timestamp        : detailed_message< 9, unsigned_<3> >     { static const char *description() { return  "Time Stamp";}};
    struct gps_position     : message<10, 14>   { static const char *description() { return  "GPS Positional Data";}};
    struct gps_raw_speed    : message<11, 10>   { static const char *description() { return  "GPS Raw Speed Data";}};
    struct beacon_pulse_present : message<12, 3>{ static const char *description() { return  "Beacon Pulse Present";}};
    struct frequency       : message_range<14, 19, 5>    { static const char *description() { return  "Frequency";}};
    struct serial_data_input: message<19, var>   { static const char *description() { return  "Serial Data Input";}};
    struct analogue        : message_range<20,52, 4>    { static const char *description() { return  "Analogue 1";}};
    struct channel_data     : message<52, 67>   { static const char *description() { return  "Channel Data Channel";}};
    struct display_data     : message<53, 11>   { static const char *description() { return  "Display Data Channel";}};
    struct reflash          : message<54, 6>    { static const char *description() { return  "Reflash Channel";}};
    struct date_storage     : message<55, 10>   { static const char *description() { return  "Date Storage Channel";}};
    struct gps_course       : message<56, 10>   { static const char *description() { return  "GPS Course Data";}};
    struct gps_accuracy     : message<57, 10>   { static const char *description() { return  "GPS Altitude and Speed Accuracy";}};
    struct extended_frequency : message_range<58, 62, 11>{ static const char *description() { return  "Extended Frequency 1";}};
    struct extended_rpm     : message<62, 11>   { static const char *description() { return  "Extended RPM";}};
    struct start_of_run     : message<63, 3>    { static const char *description() { return  "Start of Run Channel";}};
    struct processed_speed  : message<64, 5>    { static const char *description() { return  "Processed Speed Data";}};
    struct gear_setup       : message<65, 30>   { static const char *description() { return  "Gear Set Up Data";}};
    struct bargraph_setup   : message<66, 11>   { static const char *description() { return  "Bargraph Set Up Data";}};
    struct dashboard_setup  : message<67, 4>    { static const char *description() { return  "Dashboard Set Up Data";}};
    struct dashboard_setup2 : message<68, 4>    { static const char *description() { return  "Dashboard Set Up Data Two";}};
    struct new_target_sector_time : message<69, 42> { static const char *description() { return  "New Target Sector Time";}};
    struct new_target_marker_time : message<70, 42> { static const char *description() { return  "New Target Marker Time";}};
    struct auxiliary_input  : message<71, 3>    { static const char *description() { return  "Auxiliary Input Module Number";}};
    struct external_temperature : message<72, 5>{ static const char *description() { return  "External Temperature Channel";}};
    struct external_frequency: message<73, 5>   { static const char *description() { return  "External Frequency Channel";}};
    struct external_percentage : message<74, 5> { static const char *description() { return  "External Percentage Channel";}};
    struct external_time    : message<75, 6>    { static const char *description() { return  "External Time Channel";}};
    struct new_lcd_data     : message<76, 24>   { static const char *description() { return  "New LCD Data Channel";}};
    struct new_led_data     : message<77, 3>    { static const char *description() { return  "New LED Data Channel";}};
    struct precalc_distance_data : message<78, 6>{ static const char *description() { return  "Pre Calculated Distance Data Channel";}};
    struct yaw_rates        : message<79, 4>    { static const char *description() { return  "Yaw Rates Channel";}};
    struct calculated_yaw   : message<80, 5>    { static const char *description() { return  "Calculated Yaw Channel";}};
    struct pitch_rate       : message<81, 5>    { static const char *description() { return  "Pitch Rate Channel";}};
    struct pitch_angle      : message<82, 5>    { static const char *description() { return  "Pitch Angle Channel";}};
    struct roll_rate        : message<83, 5>    { static const char *description() { return  "Roll Rate Channel";}};
    struct roll_angle       : message<84, 5>    { static const char *description() { return  "Roll Angle Channel";}};
    struct gradient         : message<85, 10>   { static const char *description() { return  "Gradient Channel";}};
    struct pulse_count     : message_range<86, 90, 5>    { static const char *description() { return  "Pulse Count 1";}};
    struct baseline         : message<90, 6>    { static const char *description() { return  "Baseline Channel";}};
    struct unit_control     : message<91, 5>    { static const char *description() { return  "Unit Control Channel";}};
    struct z_acceleration   : message<92, 4>    { static const char *description() { return  "Z Acceleration";}};
    struct external_angle   : message<93, 5>    { static const char *description() { return  "External Angle Channel";}};
    struct external_pressure: message<94, 6>    { static const char *description() { return  "External Pressure Channel";}};
    struct external_misc    : message<95, 5>    { static const char *description() { return  "External Miscellaneous Channel";}};
    struct time_into_current_lap : message<96, 10>  { static const char *description() { return  "Time in to current lap and sector";}};
    struct high_res_timer   : message<97, 8>    { static const char *description() { return  "High resolution event timer";}};
    struct sector_definition: message<101, 19>  { static const char *description() { return  "Sector Definition Channel";}};
    struct brakebox_to_pc   : message<102, var>  { static const char *description() { return  "BRAKEBOX to PC Communication Channel";}};
    struct dvr_communication: message<103, 17>  { static const char *description() { return  "DVR Communication Channel";}};
    struct video_frame_index: message<104, 9>   { static const char *description() { return  "Video frame index";}};
    struct local_ned_velocities : message<105, 11>  { static const char *description() { return  "Local NED velocities";}};
    struct general_configuration : message<107, var> { static const char *description() { return  "General Configuration Message";}};

    typedef boost::mpl::vector<
            run_information ,
            run_start_stop  ,
            raw_gps         ,
            new_sector_time ,
            new_lap_marker  ,
            logger_storage  ,
            gps_time_storage    ,
            accelerations   ,
            timestamp       ,
            gps_position        ,
            gps_raw_speed   ,
            beacon_pulse_present ,
            frequency      ,
            serial_data_input,
            analogue       ,
            channel_data        ,
            display_data        ,
            reflash
            > commands1;

    typedef boost::mpl::vector<
            date_storage        ,
            gps_course      ,
            gps_accuracy        ,
            extended_frequency ,
            extended_rpm        ,
            start_of_run        ,
            processed_speed ,
            gear_setup      ,
            bargraph_setup,
            dashboard_setup ,
            dashboard_setup2 ,
            new_target_sector_time ,
            new_target_marker_time ,
            auxiliary_input ,
            external_temperature ,
            external_frequency,
            external_percentage ,
            external_time   ,
            new_lcd_data
            > commands2;

    typedef boost::mpl::vector<
            new_led_data        ,
            precalc_distance_data ,
            yaw_rates       ,
            calculated_yaw  ,
            pitch_rate      ,
            pitch_angle     ,
            roll_rate       ,
            roll_angle      ,
            gradient            ,
            pulse_count        ,
            baseline            ,
            unit_control        ,
            z_acceleration  ,
            external_angle  ,
            external_pressure,
            external_misc   ,
            time_into_current_lap ,
            high_res_timer  ,
            sector_definition
            > commands3;

    typedef boost::mpl::vector<
            brakebox_to_pc  ,
            dvr_communication,
            video_frame_index,
            local_ned_velocities ,
            general_configuration
            > commands4;


    typedef joint_view< joint_view<commands1, commands2>, joint_view<commands3, commands4> > list;
};
}


#endif /* MESSAGES_HPP_ */
