/*
 * text_printer.hpp
 *
 *  Created on: Sep 28, 2011
 *      Author: danny
 */

#ifndef TEXT_PRINTER_HPP_
#define TEXT_PRINTER_HPP_
#include <ostream>

/**
 * This class writes the message out in a csv-format (tab-separated, actually).
 */
struct text_printer : public rtlogs::messages_definition
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


#endif /* TEXT_PRINTER_HPP_ */
