//
//  Copyright (C) 2021 Danny Havenith
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include "register_all_tools.hpp"
#include "toolregistry.hpp"

class timestamp_printer_tool;
class kml_writer_tool;
class text_printer_tool;
class timestamp_correction_tool;
class gps_timestamp_printer_tool;
class data_at_event_tool;
class tnoify_tool;
class timestamp_reporter_tool;
class interpolate;
class histogram_counter_tool;
class clean_file_writer_tool;
class analogue_channel_table_tool;

void register_all_tools()
{
    register_tool<kml_writer_tool>();
    register_tool<timestamp_printer_tool>();
    register_tool<text_printer_tool>();
    register_tool<gps_timestamp_printer_tool>();
    register_tool<data_at_event_tool>();
    register_tool<tnoify_tool>();
    register_tool<timestamp_reporter_tool>();
    register_tool<interpolate>();
    register_tool<histogram_counter_tool>();
    register_tool<clean_file_writer_tool>();
    register_tool<analogue_channel_table_tool>();
}

