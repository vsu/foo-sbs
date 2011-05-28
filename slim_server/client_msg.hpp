//
// client_msg.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#ifndef SLIM_CLIENT_MSG_HPP
#define SLIM_CLIENT_MSG_HPP

#include <string>
#include <vector>

namespace slim
{
namespace server
{

/// A message received from a client.
struct client_msg
{
    /// The command string.
    std::string command;

    /// The length of the packet.
    int length;

    /// The command data.
    std::vector<char> data;

    /// Clears all fields of the client message.
    void clear();

    /// Gets the device name from the device code.
    std::string get_device_name();

    /// Gets the device firmware revision.
    char get_firmware_revision();

    /// Gets the device MAC address.
    std::vector<unsigned char> get_mac_address();
};

} // namespace server
} // namespace slim

#endif // SLIM_CLIENT_MSG_HPP