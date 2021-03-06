#ifndef BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_PORT_HPP_20100618
#define BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_PORT_HPP_20100618

// Copyright 2010 (c) Dean Michael Berris.
// Copyright 2010 (c) Sinefunc, Inc.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/optional.hpp>

namespace boost { namespace network { namespace http {

    template <class Tag>
    struct basic_request;

    namespace impl {

        template <class Tag>
        struct port_wrapper {
            basic_request<Tag> const & message_;

            port_wrapper(basic_request<Tag> const & message)
                : message_(message) {}

            typedef typename basic_request<Tag>::port_type port_type;

            operator port_type() const {
                return message_.port();
            }

            operator boost::optional<boost::uint16_t> () const {
                return uri::port_us(message_.uri());
            }
        };

    } // namespace impl

    template <class Tag>
    inline
    impl::port_wrapper<Tag>
    port(basic_request<Tag> const & request) {
        return impl::port_wrapper<Tag>(request);
    }

} // namespace http

} // namespace network

} // namespace boost

#endif // BOOST_NETWORK_PROTOCOL_HTTP_MESSAGE_PORT_HPP_20100618
