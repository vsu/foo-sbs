//
// server_msg.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#include "client_msg.hpp"

namespace slim
{
namespace server
{

void client_msg::clear()
{
    command.clear();
    length = 0;
    data.clear();
}

std::string client_msg::get_device_name()
{
    std::string device_name = "";

    if (data.size() > 0)
    {
        switch (data[0])
        {
        case 2:
            device_name = "Squeezebox";
            break;

        case 3:
            device_name = "Softsqueeze";
            break;

        case 4:
            device_name = "Squeezebox 2";
            break;

        case 5:
            device_name = "Transporter";
            break;

        case 6:
            device_name = "Softsqueeze 3";
            break;

        case 7:
            device_name = "Receiver";
            break;

        case 8:
            device_name = "Squeezeslave";
            break;

        case 9:
            device_name = "Controller";
            break;

        case 10:
            device_name = "Boom";
            break;

        case 11:
            device_name = "Softboom";
            break;

        case 12:
            device_name = "Squeezeplay";
            break;
        }
    }

    return device_name;
}

char client_msg::get_firmware_revision()
{
    return (data.size() > 1) ? data[1] : 0;
}

std::vector<unsigned char> client_msg::get_mac_address()
{
    std::vector<unsigned char> buffer;

    if (data.size() > 7)
    {
        for (int ix = 2; ix < 8; ix++)
        {
            buffer.push_back((unsigned char)data[ix]);
        }
    }

    return buffer;
}


} // namespace server
} // namespace slim