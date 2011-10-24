//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "connection_manager.hpp"
#include "request_handler.hpp"

namespace http
{
namespace server
{

connection::connection(boost::asio::io_service& io_service,
                       connection_manager& manager, request_handler& handler)
    : socket_(io_service),
      connection_manager_(manager),
      request_handler_(handler),
      ready_for_stream_(false)
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
    socket_.close();
}

int connection::send_data(void * data, size_t length)
{
    if (ready_for_stream_)
    {
        return send(native_socket_, (char *)data, length, 0);
    }

    return 0;
}

std::string connection::get_mac_address()
{
    if (!url_params_.empty())
    {
        // split URL parameters
        std::vector<std::string> params;
        
        boost::algorithm::split(
            params, 
            url_params_, 
            boost::is_any_of("&"), 
            boost::algorithm::token_compress_on);
        
        std::vector<std::string>::const_iterator it;
        for (it = params.begin(); it != params.end(); ++it)
        {
            // separate key and value fields
            std::size_t first_eq_pos = it->find_first_of("=");
            if (first_eq_pos != std::string::npos)
            {
                std::string key = it->substr(0, first_eq_pos);
                std::string value = it->substr(first_eq_pos + 1);

                if (key == "player")
                {
                    return value;
                }
            }
        }
    }

    return "";
}

void connection::handle_read(const boost::system::error_code& e,
                             std::size_t bytes_transferred)
{
    if (!e)
    {
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
                    request_, buffer_.data(), buffer_.data() + bytes_transferred);

        if (result)
        {
            request_handler_.handle_request(request_, reply_, url_params_);

            if (reply_.keep_open)
            {
                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         boost::bind(&connection::handle_write_stream, shared_from_this(),
                                                     boost::asio::placeholders::error));
            }
            else
            {
                boost::asio::async_write(socket_, reply_.to_buffers(),
                                         boost::bind(&connection::handle_write, shared_from_this(),
                                                     boost::asio::placeholders::error));
            }
        }
        else if (!result)
        {
            reply_ = reply::stock_reply(reply::bad_request);

            boost::asio::async_write(socket_, reply_.to_buffers(),
                                     boost::bind(&connection::handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error));
        }
        else
        {
            socket_.async_read_some(boost::asio::buffer(buffer_),
                                    boost::bind(&connection::handle_read, shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        connection_manager_.stop(shared_from_this());
    }
}

void connection::handle_write(const boost::system::error_code& e)
{
    if (!e)
    {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    }

    if (e != boost::asio::error::operation_aborted)
    {
        connection_manager_.stop(shared_from_this());
    }
}

void connection::handle_write_stream(const boost::system::error_code& e)
{
    if (!e)
    {
        ready_for_stream_ = true;

        native_socket_ = socket_.native();
        if (native_socket_ != INVALID_SOCKET)
        {
            int optval = 1000;
            setsockopt(native_socket_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&optval, sizeof(optval));
            setsockopt(native_socket_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&optval, sizeof(optval));
        }
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        connection_manager_.stop(shared_from_this());
    }
}

} // namespace server
} // namespace http