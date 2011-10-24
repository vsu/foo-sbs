//
// server.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SLIM_SERVER_HPP
#define SLIM_SERVER_HPP

#include <boost/asio.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "connection_manager.hpp"

namespace slim
{
namespace server
{

/// The top-level class of the HTTP server.
class server
    : private boost::noncopyable
{
public:
    /// Construct the server to listen on the specified TCP address and port.
    explicit server(const std::string& address, const int port);

    /// Run the server's io_service loop.
    void run();

    /// Stop the server.
    void stop();

    /// Returns a reference to the connection manager.
    connection_manager& get_connection_manager();

private:
    /// Handle completion of an asynchronous accept operation.
    void handle_accept(const boost::system::error_code& e);

    /// Handle a request to stop the server.
    void handle_stop();

    /// The io_service used to perform asynchronous operations.
    boost::asio::io_service io_service_;

    /// Acceptor used to listen for incoming connections.
    boost::asio::ip::tcp::acceptor acceptor_;

    /// The connection manager which owns all live connections.
    connection_manager connection_manager_;

    /// The next connection to be accepted.
    connection_ptr new_connection_;
};

} // namespace server
} // namespace slim

#endif // SLIM_SERVER_HPP