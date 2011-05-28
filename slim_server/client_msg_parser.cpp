//
// client_msg_parser.cpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#include "client_msg_parser.hpp"
#include "client_msg.hpp"

namespace slim
{
namespace server
{

client_msg_parser::client_msg_parser()
    : index_(0)
{
}

void client_msg_parser::reset()
{
    index_ = 0;
}

boost::tribool client_msg_parser::consume(client_msg& c_msg, char input)
{
    if (index_ < 4)
    {
        c_msg.command.push_back(input);
        index_++;

        if (index_ == 4)
        {
            c_msg.length = 0;
        }

        return boost::indeterminate;
    }
    else if (index_ < 8)
    {
        c_msg.length = (c_msg.length << 8) + input;
        index_++;

        if (index_ == 8)
        {
            if (c_msg.length == 0)
            {
                return true;
            }
        }

        return boost::indeterminate;
    }
    else if ((index_ - 8) < c_msg.length)
    {
        c_msg.data.push_back(input);
        index_++;

        if ((index_ - 8) == c_msg.length)
        {
            return true;
        }

        return boost::indeterminate;
    }

    return false;
}

} // namespace slim
} // namespace http