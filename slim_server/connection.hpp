//
// connection.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor Su
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SLIM_CONNECTION_HPP
#define SLIM_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "server_msg.hpp"
#include "client_msg.hpp"
#include "client_msg_parser.hpp"

#define STATUS_POLL_SEC  10
#define DISCONNECT_COUNT  3

namespace slim
{
namespace server
{

class connection_manager;

/// Represents a single connection from a client.
class connection
    : public boost::enable_shared_from_this<connection>,
  private boost::noncopyable
{
public:
    /// Construct a connection with the given io_service.
    explicit connection(boost::asio::io_service& io_service,
                        connection_manager& manager, 
                        const std::string& status_filename);

    /// Get the socket associated with the connection.
    boost::asio::ip::tcp::socket& socket();

    /// Start the first asynchronous operation for the connection.
    void start();

    /// Stop all asynchronous operations associated with the connection.
    void stop();

    /// Send a stream status command.
    void send_stream_status();

    /// Send a stream play command.
    void send_stream_play(const std::string url,
                          unsigned short int stream_port, 
                          int bits_per_sample, 
                          int sample_rate);

    /// Send a stream pause command.
    void send_stream_pause();

    /// Send a stream unpause command.
    void send_stream_unpause();

    /// Send a stream stop command.
    void send_stream_stop();

private:
    /// Handles an incoming client message.
    void handle_client_msg(client_msg& c_msg);

    /// Handle completion of a read operation.
    void handle_read(const boost::system::error_code& e,
                     std::size_t bytes_transferred);

    /// Configures the native socket.
    void configure_socket();

    /// Sends data to the native socket.
    bool send_data(std::vector<char> data);

    /// Handle status timer timeout.
    void handle_status_timer(const boost::system::error_code& e);

    /// Disconnects the client connection.
    void disconnect_client();

    /// Adds the client to the HTTP server index file.
    void html_add_client(const std::string device_name, char firmware_revision);

    /// Deletes the client from the HTTP server index file.
    void html_delete_client();

    /// Socket for the connection.
    boost::asio::ip::tcp::socket socket_;

    /// The manager for this connection.
    connection_manager& connection_manager_;

    /// Buffer for incoming data.
    boost::array<char, 8192> buffer_;

    /// The incoming client message.
    client_msg client_msg_;

    /// The parser for the client message.
    client_msg_parser client_msg_parser_;

    /// The client status filename.
    std::string status_filename_;

    /// The underlying native socket.
    SOCKET native_socket_;

    /// The status timer.
    boost::asio::deadline_timer status_timer_;

    /// Indicates whether the status timer is active.
    bool status_timer_enabled_;

    /// The device disconnection counter.
    int disconnection_counter_;

    /// The device MAC address.
    std::string mac_address_;

    /// Indicates whether the device is ready to receive commands.
    bool device_initialized_;
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace server
} // namespace slim

#endif // SLIM_CONNECTION_HPP