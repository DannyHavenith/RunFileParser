/*
 * island_removal.hpp
 *
 *  Created on: Jun 11, 2012
 *      Author: danny
 */

#ifndef ISLAND_REMOVAL_HPP_
#define ISLAND_REMOVAL_HPP_
#include <vector>

/**
 * This class is a generic base class that implements buffering of messages.
 * Instances of this class keep a reference to a 'downstream' message handler. They can store messages in a buffer and send
 * the stored messages to the downstream handler at a later time.
 */
template< typename output_handler>
class buffering_message_handler
{
protected:
    buffering_message_handler( output_handler &handler)
    : handler( handler)
    {}

    /// add a range of bytes to the buffer.
    template<typename iterator>
    void add_to_buffer( iterator begin, iterator end)
    {
        std::copy( begin, end, std::back_inserter( buffer));
    }

    /**
     * Send all bytes in the buffer to the handler class
     */
    void flush()
    {
        rtlogs::scan_log( handler, buffer.begin(), buffer.end());
        buffer.clear();
    }

    template< typename message, typename iterator>
    void bypass_buffer( message, iterator begin, iterator end)
    {
        handler.handle( message(), begin, end);
    }

    void clear()
    {
        buffer.clear();
    }


private:
    typedef std::vector<unsigned char> buffer_type;
    buffer_type     buffer;
    output_handler  &handler;
};

/**
 * This class receives RT messages and passes them on to an 'output_handler' class.
 * This class will filter out all unparseable data and all 'islands', where an island is a single
 * message surrounded by unparseable data.
 */
template <typename output_handler>
class island_remover : private buffering_message_handler<output_handler>
{
public:
    island_remover( output_handler &handler)
    :buffering_message_handler<output_handler>( handler), last_was_error( true)
    {}

    /**
     * Handle a valid message.
     * The message will be sent straight to the downstream handler if the previous message was also valid.
     * If the previous message was not parsed correctly (did not have a valid checksum), the message will be buffered and will
     * only be sent through if a valid message follows this one.
     */
    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {
        if (last_was_error)
        {
            add_to_buffer( begin, end);
        }
        else
        {
            flush();
            bypass_buffer( message(), begin, end);
        }
        last_was_error = false;
    }

    /**
     * Handle bytes with an invalid checksum.
     * If there was a message in the buffer, it will be discarded. Any following message will be
     * stored in the buffer.
     */
    template< typename iterator>
    void handle( rtlogs::parse_error, iterator , iterator )
    {
        buffer_type::clear(); // remove any message that was in the buffer.
        last_was_error = true;
    }

    void flush()
    {
        buffer_type::flush();
    }

private:
    typedef buffering_message_handler<output_handler> buffer_type;
    bool            last_was_error;
};


#endif /* ISLAND_REMOVAL_HPP_ */
