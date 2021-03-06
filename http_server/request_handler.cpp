//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include "../foo_sbs/defs.h"
#include "../foo_sbs/shm.h"

namespace http
{
namespace server
{

request_handler::request_handler(const std::string& doc_root)
    : doc_root_(doc_root)
{
}

void request_handler::handle_request(const request& req, reply& rep, std::string& url_params)
{
    // Decode url to path.
    std::string request_path;
    if (!url_decode(req.uri, request_path))
    {
        rep = reply::stock_reply(reply::bad_request);
        return;
    }

    // Remove and save any url parameters
    std::size_t first_qm_pos = request_path.find_first_of("?");
    if (first_qm_pos != std::string::npos)
    {
        url_params = request_path.substr(first_qm_pos + 1);
        request_path = request_path.substr(0, first_qm_pos);
    }

    // Request path must be absolute and not contain "..".
    if (request_path.empty() || request_path[0] != '/'
            || request_path.find("..") != std::string::npos)
    {
        rep = reply::stock_reply(reply::bad_request);
        return;
    }

    // Determine the file extension.
    std::size_t last_slash_pos = request_path.find_last_of("/");
    std::size_t last_dot_pos = request_path.find_last_of(".");
    std::string extension;
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
    {
        extension = request_path.substr(last_dot_pos + 1);
    }

    if (request_path == URL_PATH_STREAM)
    {
        // Fill out the reply to be sent to the client.
        rep.status = reply::ok;
        rep.headers.resize(2);
        rep.headers[0].name = "Connection";
        rep.headers[0].value = "close";
        rep.headers[1].name = "Content-Type";
        rep.headers[1].value = "audio/x-pcm";
        rep.keep_open = true;
    }
    else
    {
        // Fill out the reply to be sent to the client.
        rep.status = reply::ok;

        rep.content.append( 
            "<html>"
            "<head><title>Foobar Squeezebox Server</title></head>"
            "<body><h3>Active Clients</h3><div>");

        // Open or create the named mutex
        named_mutex mutex(open_or_create, FOO_SBS_MUTEX);
        scoped_lock<named_mutex> lock(mutex);

        // Open the managed segment
        managed_shared_memory segment(open_only, FOO_SBS_SHM_SEGMENT);  

        // Find the map using the c-string name
        ShMemMap *shm_map = segment.find<ShMemMap>(FOO_SBS_SHM_MAP).first;
        
        // Iterate the map
        ShMemMap::const_iterator it;
        for (it = shm_map->begin(); it != shm_map->end(); ++it)
        {
            rep.content.append("<span>" + it->second + "</span><br />");
        }

        rep.content.append("</div></body></html>");

        rep.headers.resize(2);
        rep.headers[0].name = "Content-Length";
        rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
        rep.headers[1].value = mime_types::extension_to_type(extension);
        rep.keep_open = false;
    }
    /*** Disable file retrieval
    else
    {
        if (!doc_root_.empty())
        {
            // Open the file to send back.
            std::string full_path = doc_root_ + request_path;
            std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
            if (!is)
            {
                rep = reply::stock_reply(reply::not_found);
                return;
            }

            // Fill out the reply to be sent to the client.
            rep.status = reply::ok;
            char buf[512];
            while (is.read(buf, sizeof(buf)).gcount() > 0)
                rep.content.append(buf, is.gcount());
            rep.headers.resize(2);
            rep.headers[0].name = "Content-Length";
            rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
            rep.headers[1].value = mime_types::extension_to_type(extension);
            rep.keep_open = false;
        }
        else
        {
            rep = reply::stock_reply(reply::not_found);
        }
    }
    ***/
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i)
    {
        if (in[i] == '%')
        {
            if (i + 3 <= in.size())
            {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value)
                {
                    out += static_cast<char>(value);
                    i += 2;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        else if (in[i] == '+')
        {
            out += ' ';
        }
        else
        {
            out += in[i];
        }
    }
    return true;
}

} // namespace server
} // namespace http