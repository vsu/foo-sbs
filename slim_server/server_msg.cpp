//
// server_msg.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#include "server_msg.hpp"

namespace slim {
namespace server {

server_msg_aude::server_msg_aude()
{
	length_ = 6;

	command_[0] = 'a';
	command_[1] = 'u';
	command_[2] = 'd';	
	command_[3] = 'e';
}

std::vector<char> server_msg_aude::to_vector()
{
	std::vector<char> buffer;

	// Put length in network order
	buffer.push_back((char)(length_ >> 8));
	buffer.push_back((char)length_);

	buffer.push_back(command_[0]);
	buffer.push_back(command_[1]);
	buffer.push_back(command_[2]);
	buffer.push_back(command_[3]);

	buffer.push_back(spdif_enable ? 1 : 0);
	buffer.push_back(dac_enable ? 1 : 0);

	return buffer;
}

server_msg_audg::server_msg_audg()
{
	length_ = 22;

	command_[0] = 'a';
	command_[1] = 'u';
	command_[2] = 'd';	
	command_[3] = 'g';
}

std::vector<char> server_msg_audg::to_vector()
{
	std::vector<char> buffer;

	// Put length in network order
	buffer.push_back((char)(length_ >> 8));
	buffer.push_back((char)length_);

	buffer.push_back(command_[0]);
	buffer.push_back(command_[1]);
	buffer.push_back(command_[2]);
	buffer.push_back(command_[3]);

	unsigned char vol = (volume > 128) ? 128 : volume;

	// Old style volume setting (2 bytes short int)

	// Left channel
	buffer.push_back(0);
	buffer.push_back(0);
	buffer.push_back(0);
	buffer.push_back(vol);

	// Right channel
	buffer.push_back(0);
	buffer.push_back(0);
	buffer.push_back(0);
	buffer.push_back(vol);

	buffer.push_back(dvc_enable ? 1 : 0);
	buffer.push_back(preamp);

	// New style volume setting (2 bytes 16.16 fixed point)
	
	if (vol == 128)
	{
		// Left channel
		buffer.push_back(0);
		buffer.push_back(1);
		buffer.push_back(0);
		buffer.push_back(0);

		// Right channel
		buffer.push_back(0);
		buffer.push_back(1);
		buffer.push_back(0);
		buffer.push_back(0);
	}
	else
	{
		short int value = (short int)(65535 * vol / 128);

		// Left channel
		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back((char)(value >> 8));
		buffer.push_back((char)value);

		// Right channel
		buffer.push_back(0);
		buffer.push_back(0);
		buffer.push_back((char)(value >> 8));
		buffer.push_back((char)value);
	}

	return buffer;
}

server_msg_strm::server_msg_strm()
{
	command_[0] = 's';
	command_[1] = 't';
	command_[2] = 'r';
	command_[3] = 'm';
}

std::vector<char> server_msg_strm::to_vector()
{
	std::vector<char> buffer;

	// Put placeholders for length
	buffer.push_back(0);
	buffer.push_back(0);

	buffer.push_back(command_[0]);
	buffer.push_back(command_[1]);
	buffer.push_back(command_[2]);
	buffer.push_back(command_[3]);

	buffer.push_back(strm_command);
	buffer.push_back(auto_start);
	buffer.push_back(format_byte);
	buffer.push_back(pcm_sample_size);
	buffer.push_back(pcm_sample_rate);
	buffer.push_back(pcm_channels);
	buffer.push_back(pcm_endian);
	buffer.push_back(threshold);
	buffer.push_back(spdif_enable);
	buffer.push_back(trans_period);
	buffer.push_back(trans_type);
	buffer.push_back(flags);
	buffer.push_back(output_threshold);
	buffer.push_back(0);

	buffer.push_back(replay_gain[0]);
	buffer.push_back(replay_gain[1]);
	buffer.push_back(replay_gain[2]);
	buffer.push_back(replay_gain[3]);

	buffer.push_back((char)(server_port >> 8));
	buffer.push_back((char)server_port);

	buffer.push_back(server_ip_address[0]);
	buffer.push_back(server_ip_address[1]);
	buffer.push_back(server_ip_address[2]);
	buffer.push_back(server_ip_address[3]);

	if (stream_url != "")
	{
		std::string http_header;
		std::stringstream out;

		out << "GET " << stream_url << " HTTP/1.0\r\n\r\n";
		http_header = out.str();

		std::string::iterator c;
		for (c = http_header.begin(); c != http_header.end(); ++c)
		{
			buffer.push_back(*c);
		}
	}

	// Length is data plus command
	length_ = buffer.size() - 2;
	buffer[0] = (char)(length_ >> 8);
	buffer[1] = (char)length_;

	return buffer;
}

char server_msg_strm::get_pcm_sample_size(int bits_per_sample)
{
	char pcm_sample_size = '?';

	switch (bits_per_sample)
	{
	case 16:
		pcm_sample_size = '1';
		break;

	case 24:
		pcm_sample_size = '2';
		break;
	}

	return pcm_sample_size;
}

char server_msg_strm::get_pcm_sample_rate(int sample_rate)
{
	char pcm_sample_rate = '?';

	switch (sample_rate)
	{
	case 44100:
		pcm_sample_rate = '3';
		break;

	case 48000:
		pcm_sample_rate = '4';
		break;

	case 96000:
		pcm_sample_rate = '9';
		break;
	}

	return pcm_sample_rate;
}

} // namespace server
} // namespace slim