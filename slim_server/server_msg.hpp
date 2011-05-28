//
// server_msg.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#ifndef SLIM_SERVER_MSG_HPP
#define SLIM_SERVER_MSG_HPP

#include <string>
#include <sstream>
#include <vector>
#include <boost/asio.hpp>

namespace slim
{
namespace server
{

/// An audio output command server message.
class server_msg_aude
{
public:
    /// Constructor.
    server_msg_aude();

    /// Convert the server message into a char vector.
    std::vector<char> to_vector();

    /// Enable or disable SPDIF output.
    bool spdif_enable;

    /// Enable or disable DAC output.
    bool dac_enable;

private:
    /// The length of the message in bytes (in network order) not including
    /// the length itself.
    int length_;

    /// A 4 byte command header.
    char command_[4];
};


/// An audio gain command server message.
class server_msg_audg
{
public:
    /// Constructor.
    server_msg_audg();

    /// Convert the server message into a char vector.
    std::vector<char> to_vector();

    /// The volume level from 0 to 128.
    unsigned char volume;

    /// Enable or disable volume control.
    bool dvc_enable;

    /// The preamp gain from 0 to 255.
    unsigned char preamp;

private:
    /// The length of the message in bytes (in network order) not including
    /// the length itself.
    int length_;

    /// A 4 byte command header.
    char command_[4];
};


/// A stream command server message.
struct server_msg_strm
{
public:
    /// Constructor.
    server_msg_strm();

    /// Convert the server message into a char vector.
    std::vector<char> to_vector();

    /// The stream command byte.
    /// 's' start, 'p' pause, 'u' unpause, 'q' stop, 't' status, 'f' flush, 'a' skip-ahead
    char strm_command;

    /// The autostart parameter.
    /// '0' = don't auto-start, '1' = auto-start, '2' = direct streaming, '3' = direct+auto
    char auto_start;

    /// The format byte.
    /// 'p' = PCM, 'm' = MP3, 'f' = FLAC, 'w' = WMA, 'o' = Ogg., 'a' = AAC (& HE-AAC), 'l' = ALAC
    char format_byte;

    /// The PCM sample size.
    /// '0' = 8, '1' = 16, '2' = 20, '3' = 32; usually '1', '?' for self-describing formats.
    char pcm_sample_size;

    /// The PCM sample rate.
    /// '0'=11kHz, '1'=22kHz, '2'=32kHz, '3'=44.1kHz, '4'=48kHz, '5'=8kHz, '6'=12kHz, '7'=16kHz,
    /// '8'=24kHz, '9'=96kHz; usually 3, '?' for self-describing formats.
    char pcm_sample_rate;

    /// The number of PCM channels.
    /// '1'=mono, '2'=stereo; usually '2', '?' for self-describing formats.
    char pcm_channels;

    /// The PCM endianness.
    /// '0' = big, '1' = little; '1' for WAV, '0' for AIFF, '?' for self-describing formats.
    char pcm_endian;

    /// <summary>
    /// KB of input buffer data before autostart or notify.
    /// </summary>
    unsigned char threshold;

    /// Controls the SPDIF output.
    /// '0'=auto, '1'=on, '2'=off; usually 0
    char spdif_enable;

    /// Transition duration in seconds.
    unsigned char trans_period;

    /// The transition type.
    /// '0' = none, '1' = crossfade,'2' = fade in, '3' = fade out, '4' = fade in & fade out
    char trans_type;

    /// Playback flags.
    /// 0x80 - loop infinitely, 0x40 - stream without restarting decoder,
    /// 0x01 - polarity inversion left, 0x02 - polarity inversion right
    unsigned char flags;

    /// Amount of output buffer data before playback starts, in tenths of second.
    unsigned char output_threshold;

    /// The replay gain.
    /// replay gain in 16.16 fixed point, 0 = none
    unsigned char replay_gain[4];

    /// The server port to use.
    unsigned short int server_port;

    /// The IP address of the server (0 means use IP of control server).
    unsigned char server_ip_address[4];

    /// The stream URL string.
    std::string stream_url;

    /// Returns the pcm_sample_size code for a given bits per sample value.
    char get_pcm_sample_size(int bits_per_sample);

    /// Returns the pcm_sample_rate code for a given sample rate.
    char get_pcm_sample_rate(int sample_rate);

private:
    /// The length of the message in bytes (in network order) not including
    /// the length itself.
    int length_;

    /// A 4 byte command header.
    char command_[4];
};

} // namespace server
} // namespace slim

#endif // SLIM_SERVER_MSG_HPP