/*
 * detail_handler.hpp
 *
 *  Created on: Sep 29, 2011
 *      Author: danny
 */

#ifndef DETAIL_HANDLER_HPP_
#define DETAIL_HANDLER_HPP_

template< typename Derived>
struct detail_handler
{
    Derived &derived_this() { return static_cast< Derived&>(*this);}
    const Derived &derived_this() const  { return static_cast< const Derived&>(*this);}

    template< typename message, typename iterator>
    void handle( message, iterator begin, iterator end)
    {

    }

};


#endif /* DETAIL_HANDLER_HPP_ */
