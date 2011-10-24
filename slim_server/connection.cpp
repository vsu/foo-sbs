//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor Su
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include "connection_manager.hpp"
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include "../foo_sbs/shm.h"

namespace slim
{
namespace server
{

connection::connection(boost::asio::io_service& io_service,
                       connection_manager& connection_manager)
    : socket_(io_service),
      connection_manager_(connection_manager),
      status_timer_(io_service),
      status_timer_enabled_(false),
      mac_address_(""),
      device_initialized_(false)
{
}

boost::asio::ip::tcp::socket& connection::socket()
{
    return socket_;
}

void connection::start()
{
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            boost::bind(&connection::handle_read, shared_from_this(),
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void connection::stop()
{
    remove_client();
    socket_.close();
}

void connection::send_stream_status()
{
    if (!device_initialized_)
    {
        return;
    }

    server_msg_strm msg;

    msg.strm_command = 't';
    msg.auto_start = '0';
    msg.format_byte = 'm';
    msg.pcm_sample_size = '?';
    msg.pcm_sample_rate = '?';
    msg.pcm_channels = '?';
    msg.pcm_endian = '?';
    msg.threshold = 0;
    msg.spdif_enable = 0;
    msg.trans_period = 0;
    msg.trans_type = '0';
    msg.flags = 0;
    msg.output_threshold = 0;

    msg.replay_gain[0] = 0;
    msg.replay_gain[1] = 0;
    msg.replay_gain[2] = 0;
    msg.replay_gain[3] = 0;

    msg.server_port = 0;

    msg.server_ip_address[0] = 0;
    msg.server_ip_address[1] = 0;
    msg.server_ip_address[2] = 0;
    msg.server_ip_address[3] = 0;

    send_data(msg.to_vector());
}

void connection::send_stream_play(const std::string url,
                                  unsigned short int stream_port, 
                                  int bits_per_sample,
                                  int sample_rate)
{
    if (!device_initialized_)
    {
        return;
    }

    server_msg_strm msg;

    msg.strm_command = 's';
    msg.auto_start = '1';
    msg.format_byte = 'p';
    msg.pcm_sample_size = msg.get_pcm_sample_size(bits_per_sample);
    msg.pcm_sample_rate = msg.get_pcm_sample_rate(sample_rate);
    msg.pcm_channels = '2';
    msg.pcm_endian = '1';
    msg.threshold = 0xff;
    msg.spdif_enable = 0;
    msg.trans_period = 0;
    msg.trans_type = '0';
    msg.flags = 0;
    msg.output_threshold = 0;

    msg.replay_gain[0] = 0;
    msg.replay_gain[1] = 0;
    msg.replay_gain[2] = 0;
    msg.replay_gain[3] = 0;

    msg.server_port = stream_port;

    msg.server_ip_address[0] = 0;
    msg.server_ip_address[1] = 0;
    msg.server_ip_address[2] = 0;
    msg.server_ip_address[3] = 0;

    msg.stream_url = url + "?player=" + mac_address_;

    send_data(msg.to_vector());
}

void connection::send_stream_pause()
{
    if (!device_initialized_)
    {
        return;
    }

    server_msg_strm msg;

    msg.strm_command = 'p';
    msg.auto_start = '0';
    msg.format_byte = 'm';
    msg.pcm_sample_size = '?';
    msg.pcm_sample_rate = '?';
    msg.pcm_channels = '?';
    msg.pcm_endian = '?';
    msg.threshold = 0;
    msg.spdif_enable = 0;
    msg.trans_period = 0;
    msg.trans_type = '0';
    msg.flags = 0;
    msg.output_threshold = 0;

    msg.replay_gain[0] = 0;
    msg.replay_gain[1] = 0;
    msg.replay_gain[2] = 0;
    msg.replay_gain[3] = 0;

    msg.server_port = 0;

    msg.server_ip_address[0] = 0;
    msg.server_ip_address[1] = 0;
    msg.server_ip_address[2] = 0;
    msg.server_ip_address[3] = 0;

    msg.stream_url = "";

    send_data(msg.to_vector());
}

void connection::send_stream_unpause()
{
    if (!device_initialized_)
    {
        return;
    }

    server_msg_strm msg;

    msg.strm_command = 'u';
    msg.auto_start = '0';
    msg.format_byte = 'm';
    msg.pcm_sample_size = '?';
    msg.pcm_sample_rate = '?';
    msg.pcm_channels = '?';
    msg.pcm_endian = '?';
    msg.threshold = 0;
    msg.spdif_enable = 0;
    msg.trans_period = 0;
    msg.trans_type = '0';
    msg.flags = 0;
    msg.output_threshold = 0;

    msg.replay_gain[0] = 0;
    msg.replay_gain[1] = 0;
    msg.replay_gain[2] = 0;
    msg.replay_gain[3] = 0;

    msg.server_port = 0;

    msg.server_ip_address[0] = 0;
    msg.server_ip_address[1] = 0;
    msg.server_ip_address[2] = 0;
    msg.server_ip_address[3] = 0;

    msg.stream_url = "";

    send_data(msg.to_vector());
}

void connection::send_stream_stop()
{
    if (!device_initialized_)
    {
        return;
    }

    server_msg_strm msg;

    msg.strm_command = 'q';
    msg.auto_start = '0';
    msg.format_byte = 'm';
    msg.pcm_sample_size = '?';
    msg.pcm_sample_rate = '?';
    msg.pcm_channels = '?';
    msg.pcm_endian = '?';
    msg.threshold = 0;
    msg.spdif_enable = 0;
    msg.trans_period = 0;
    msg.trans_type = '0';
    msg.flags = 0;
    msg.output_threshold = 0;

    msg.replay_gain[0] = 0;
    msg.replay_gain[1] = 0;
    msg.replay_gain[2] = 0;
    msg.replay_gain[3] = 0;

    msg.server_port = 0;

    msg.server_ip_address[0] = 0;
    msg.server_ip_address[1] = 0;
    msg.server_ip_address[2] = 0;
    msg.server_ip_address[3] = 0;

    msg.stream_url = "";

    send_data(msg.to_vector());
}

std::string connection::get_mac_address()
{
    return mac_address_;
}

void connection::handle_client_msg(client_msg& c_msg)
{
    if (c_msg.command == "ANIC")
    {
    }
    else if (c_msg.command == "BODY")
    {
    }
    else if (c_msg.command == "BUTN")
    {
    }
    else if (c_msg.command == "BYE!")
    {
        disconnect();
    }
    else if (c_msg.command == "DBUG")
    {
    }
    else if (c_msg.command == "DSCO")
    {
    }
    else if (c_msg.command == "HELO")
    {
        configure_socket();

        // Format and save the device MAC address
        std::vector<unsigned char> mac = c_msg.get_mac_address();

        char mac_str[18];
        sprintf_s(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        mac_address_ = std::string(mac_str);
        
        std::string client_info = 
            get_client_info(c_msg.get_device_name(), c_msg.get_firmware_revision());
        
        // Open or create the named mutex
        named_mutex mutex(open_or_create, FOO_SBS_MUTEX);
        scoped_lock<named_mutex> lock(mutex);

        // Open the managed segment
        managed_shared_memory segment(open_only, FOO_SBS_SHM_SEGMENT);  

        // Find the map using the c-string name
        ShMemMap *shm_map = segment.find<ShMemMap>(FOO_SBS_SHM_MAP).first;
        
        // Add the new client
        shm_map->erase(mac_address_);
        shm_map->insert(std::pair<std::string, std::string>(mac_address_, client_info));

        server_msg_aude msg_aude;
        msg_aude.spdif_enable = true;
        msg_aude.dac_enable = true;
        send_data(msg_aude.to_vector());

        server_msg_audg msg_audg;
        msg_audg.volume = 128;
        msg_audg.dvc_enable = false;
        msg_audg.preamp = 255;
        send_data(msg_audg.to_vector());

        device_initialized_ = true;

        disconnection_counter_ = 0;
 
        status_timer_enabled_ = true;
        status_timer_.expires_from_now(boost::posix_time::seconds(STATUS_POLL_SEC));
        status_timer_.async_wait(boost::bind(&connection::handle_status_timer, shared_from_this(),
                                 boost::asio::placeholders::error));
    }
    else if (c_msg.command == "IR  ")
    {
    }
    else if (c_msg.command == "KNOB")
    {
    }
    else if (c_msg.command == "META")
    {
    }
    else if (c_msg.command == "RAWI")
    {
    }
    else if (c_msg.command == "RESP")
    {
    }
    else if (c_msg.command == "SETD")
    {
    }
    else if (c_msg.command == "STAT")
    {
        disconnection_counter_ = 0;
    }
    else if (c_msg.command == "UREQ")
    {
    }
    else if (c_msg.command == "ALSS")
    {
    }
}

void connection::handle_read(const boost::system::error_code& e,
                             std::size_t bytes_transferred)
{
    if (!e)
    {
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = client_msg_parser_.parse(
                    client_msg_, buffer_.data(), buffer_.data() + bytes_transferred);

        if (result)
        {
            handle_client_msg(client_msg_);
            client_msg_.clear();
            client_msg_parser_.reset();
        }
        else if (!result)
        {
            client_msg_.clear();
            client_msg_parser_.reset();
        }

        socket_.async_read_some(boost::asio::buffer(buffer_),
                                boost::bind(&connection::handle_read, shared_from_this(),
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        connection_manager_.stop(shared_from_this());
    }
}

void connection::configure_socket()
{
    native_socket_ = socket_.native();
    if (native_socket_ != INVALID_SOCKET)
    {
        int optval = 1000;
        setsockopt(native_socket_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&optval, sizeof(optval));
        setsockopt(native_socket_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&optval, sizeof(optval));
    }
}

bool connection::send_data(std::vector<char> data)
{
    int length = data.size();
    char * buffer = new char[length];

    for (int ix = 0; ix < length; ix++)
    {
        buffer[ix] = data.at(ix);
    }

    bool result = (send(native_socket_, buffer, length, 0) != SOCKET_ERROR);

    delete[] buffer;
    return result;
}

void connection::handle_status_timer(const boost::system::error_code& e)
{
    if (++disconnection_counter_ == DISCONNECT_COUNT)
    {
        disconnect();
    }
    else
    {
        send_stream_status();
    }
    
    if (status_timer_enabled_)
    {
        status_timer_.expires_from_now(boost::posix_time::seconds(STATUS_POLL_SEC));
        status_timer_.async_wait(boost::bind(&connection::handle_status_timer, shared_from_this(),
                                             boost::asio::placeholders::error));
    }
}

void connection::disconnect()
{
    remove_client();
    connection_manager_.stop(shared_from_this());
}

void connection::remove_client()
{
    device_initialized_ = false;

    status_timer_.cancel();
    status_timer_enabled_ = false;

    // Open or create the named mutex
    named_mutex mutex(open_or_create, FOO_SBS_MUTEX);
    scoped_lock<named_mutex> lock(mutex);

    // Open the managed segment
    managed_shared_memory segment(open_only, FOO_SBS_SHM_SEGMENT);  

    // Find the map using the c-string name
    ShMemMap *shm_map = segment.find<ShMemMap>(FOO_SBS_SHM_MAP).first;
        
    // Remove the client
    shm_map->erase(mac_address_);

    mac_address_ = "";
}

std::string connection::get_client_info(std::string device_name, char firmware_revision)
{
    // Convert firmware revision to string
    char revision_str[4];
    sprintf_s(revision_str, 4, "%u", firmware_revision);

    // Get current local time
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    std::string now_str = boost::posix_time::to_simple_string(now);

    // Get IP address of socket
    std::string ip_addr = socket_.remote_endpoint().address().to_string();

    return (
        "[" + now_str + "] " +  mac_address_ + " | " + ip_addr + " | " + device_name + 
        " revision " + revision_str);
}

} // namespace server
} // namespace slim