//
// connection_manager.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SLIM_CONNECTION_MANAGER_HPP
#define SLIM_CONNECTION_MANAGER_HPP

#include <set>
#include <boost/noncopyable.hpp>
#include "connection.hpp"

namespace slim
{
namespace server
{

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
    : private boost::noncopyable
{
public:
    /// Add the specified connection to the manager and start it.
    void start(connection_ptr c);

    /// Stop the specified connection.
    void stop(connection_ptr c);

    /// Stop all connections.
    void stop_all();

    /// Send stream play command to all connections.
    void send_stream_play_all(const std::string url,
                              unsigned short int stream_port, 
                              int bits_per_sample, 
                              int sample_rate);

    /// Send stream pause command to all connections.
    void send_stream_pause_all();

    /// Send stream unpause command to all connections.
    void send_stream_unpause_all();

    /// Send stream stop command to all connections.
    void send_stream_stop_all();

    /// Set audio gain for all connections.
    void send_audio_gain_all(unsigned char vol, 
                             bool dvc_enable, 
                             unsigned char preamp);

    /// Send stream play command to the connection associated 
    /// with the given MAC address.
    void send_stream_play(const std::string mac_address,
                          const std::string url,
                          unsigned short int stream_port, 
                          int bits_per_sample, 
                          int sample_rate);

private:
    /// The managed connections.
    std::set<connection_ptr> connections_;
};

} // namespace server
} // namespace slim

#endif // SLIM_CONNECTION_MANAGER_HPP