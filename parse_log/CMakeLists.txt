
##          Copyright Danny Havenith 2011.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

add_executable(
	parse_log
	
	timestamp_printer.hpp
	text_printer.hpp
	clean_file_writer.hpp
	histogram_counter.hpp
	analogue_channel_table.hpp
	timestamp_reporter.hpp
	gps_time_printer.hpp
	kml_writer.hpp
	timestamp_correction.hpp

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
	)
	
target_link_libraries(parse_log ${Boost_LIBRARIES})	



