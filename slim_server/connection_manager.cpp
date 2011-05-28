//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_manager.hpp"
#include <algorithm>
#include <boost/bind.hpp>

namespace slim
{
namespace server
{

void connection_manager::start(connection_ptr c)
{
    connections_.insert(c);
    c->start();
}

void connection_manager::stop(connection_ptr c)
{
    connections_.erase(c);
    c->stop();
}

void connection_manager::stop_all()
{
    std::for_each(connections_.begin(), connections_.end(),
                  boost::bind(&connection::stop, _1));
    connections_.clear();
}

void connection_manager::send_stream_play_all(const std::string url,
        unsigned short int stream_port, int bits_per_sample, int sample_rate)
{
    std::set<connection_ptr>::const_iterator c;
    for (c = connections_.begin(); c != connections_.end(); ++c)
    {
        ((connection_ptr)(*c))->send_stream_play(url, stream_port,
                bits_per_sample, sample_rate);
    }
}

void connection_manager::send_stream_pause_all()
{
    std::set<connection_ptr>::const_iterator c;
    for (c = connections_.begin(); c != connections_.end(); ++c)
    {
        ((connection_ptr)(*c))->send_stream_pause();
    }
}

void connection_manager::send_stream_unpause_all()
{
    std::set<connection_ptr>::const_iterator c;
    for (c = connections_.begin(); c != connections_.end(); ++c)
    {
        ((connection_ptr)(*c))->send_stream_unpause();
    }
}

void connection_manager::send_stream_stop_all()
{
    std::set<connection_ptr>::const_iterator c;
    for (c = connections_.begin(); c != connections_.end(); ++c)
    {
        ((connection_ptr)(*c))->send_stream_stop();
    }
}

} // namespace server
} // namespace slim