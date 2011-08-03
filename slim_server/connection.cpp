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
#include <boost/htxml.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <sstream>
#include <vector>

namespace slim
{
namespace server
{

connection::connection(boost::asio::io_service& io_service,
                       connection_manager& manager, 
                       const std::string& status_filename)
    : socket_(io_service),
      connection_manager_(manager),
      status_filename_(status_filename),
      status_timer_(io_service),
      status_timer_enabled_(false),
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
    status_timer_.cancel();
    status_timer_enabled_ = false;
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
        disconnect_client();
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

        // Delete any existing entries with this MAC address
        html_delete_client();

        // Add a new entry for this MAC address
        html_add_client(c_msg.get_device_name(), c_msg.get_firmware_revision());

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
        disconnect_client();
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

void connection::disconnect_client()
{
    device_initialized_ = false;

    status_timer_.cancel();
    status_timer_enabled_ = false;

    html_delete_client();

    connection_manager_.stop(shared_from_this());
}

void connection::html_add_client(const std::string device_name, char firmware_revision)
{
    try
    {
        std::fstream in_file(status_filename_.c_str(), std::ios_base::in);
        boost::htxml::document html_dom;
        html_dom.read(in_file);

        boost::htxml::ptr_element_vector_t div = html_dom.getElementsByName("div");

        if (div.size() > 0)
        {
            boost::htxml::element * span = new boost::htxml::element("span", div[0]);
            boost::htxml::element * br = new boost::htxml::element("br", div[0]);

            boost::htxml::attrib_value attr;
            attr._name = "id";
            attr._value = mac_address_;
            attr._singleton = false;
            span->setAttr(attr);

            // Convert firmware revision to string
            char revision_str[4];
            sprintf_s(revision_str, 4, "%u", firmware_revision);

            // Get current local time
            boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
            std::string now_str = boost::posix_time::to_simple_string(now);

            span->setValue("[" + now_str + "] " + device_name + " revision " + revision_str);

            div[0]->addChild(boost::htxml::ptr_element_t(span));
            div[0]->addChild(boost::htxml::ptr_element_t(br));
        }

        std::ofstream out_file(status_filename_.c_str());
        html_dom.write(out_file);
        out_file.close();
    }
    catch (std::exception&)
    {
    }
}

void connection::html_delete_client()
{
    try
    {
        std::fstream html_file(status_filename_.c_str(), std::ios_base::in);
        boost::htxml::document html_dom;
        html_dom.read(html_file);

        boost::htxml::ptr_element_vector_t div = html_dom.getElementsByName("div");
        boost::htxml::ptr_element_vector_t span = html_dom.getElementsById(mac_address_);

        // There appears to be a bug in the htxml code that erases elements.
        // We will hide the elements for now.
        //if ((div.size() > 0 ) && (span.size() > 0))
        //{
        //	div[0]->erase(span.begin(), span.end());
        //}

        boost::htxml::ptr_element_vector_t::iterator iter;
        for (iter = span.begin(); iter != span.end(); ++iter)
        {
            boost::htxml::attrib_value attr;
            attr._name = "style";
            attr._value = "display:none";
            attr._singleton = false;
            (*iter)->setAttr(attr);
        }

        std::ofstream out_file(status_filename_.c_str());
        html_dom.write(out_file);
        out_file.close();
    }
    catch (std::exception&)
    {
    }
}

} // namespace server
} // namespace slim