
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

add_executable(
	parse_log
	
	analogue_channel_table.hpp
	bytes_to_numbers.hpp
	clean_file_writer.hpp
	detail_handler.hpp
	gps_time_printer.hpp
	histogram_counter.hpp
	kml_writer.hpp
	logscanner.hpp
	message.hpp
	messages.hpp
	parse_error.hpp
	text_printer.hpp
	timestamp_correction.hpp
	timestamp_printer.hpp
	timestamp_reporter.hpp
	tool_implementation.hpp
	toolregistry.hpp
	data_at_event.hpp
	
	
	parse_log.cpp
	toolregistry.cpp
	timestamp_printer.cpp
	text_printer.cpp
	clean_file_writer.cpp
	histogram_counter.cpp
	analogue_channel_table.cpp
	timestamp_reporter.cpp
	gps_time_printer.cpp
	kml_writer.cpp
	timestamp_correction.cpp
	tnoify.cpp
    data_at_event.cpp
    message_names.cpp
    interpolate.cpp
    
    register_all_tools.cpp
    csv_columns.cpp
	)

target_link_libraries(
    parse_log
    PUBLIC
        CONAN_PKG::boost
        project_options 
        )	



