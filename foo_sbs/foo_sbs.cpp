#include <winsock2.h>
#include <foobar2000.h>
#include "../ATLHelpers/ATLHelpers.h"
#include <string>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "foo_sbs.h"
#include "preferences.h"
#include "../http_server/server.hpp"
#include "../slim_server/server.hpp"

#define COMPONENT_NAME		"Squeezebox Server"
#define COMPONENT_VERSION	"0.1"

http::server::server * http_server;
boost::thread * http_server_thread = NULL;

slim::server::server * slim_server;
boost::thread * slim_server_thread = NULL;

bool send_play = false;
bool is_playing = false;

std::string app_path;


class initquit_sbs : public initquit
{
    void on_init()
    {
        app_path.append(core_api::get_profile_path());
        app_path.append("\\");
        app_path.append(core_api::get_my_file_name());

        // Remove the "file://" prefix
        app_path.erase(0, 7);

        boost::filesystem::create_directories(app_path);

        g_start_server();
        g_register_callback_sbs();
    }

    void on_quit()
    {
        g_stop_server();
        g_unregister_callback_sbs();
    }
};

static initquit_factory_t<initquit_sbs> g_initquit_sbs_factory;


class playback_stream_capture_callback_sbs : public playback_stream_capture_callback
{
public:
    void on_chunk(const audio_chunk & chunk)
    {
        if (send_play)
        {
            send_play = false;
            unsigned int srate = chunk.get_srate();

            slim_server->connection_manager_.send_stream_play_all(
                "/stream.pcm", cfg_http_port.get_value(), g_get_bps(), srate);
        }

        if (is_playing)
        {
            mem_block_container_impl_t<> data;
            chunk.to_raw_data(data, g_get_bps());
            http_server->connection_manager_.send_data_all(data.get_ptr(), data.get_size());
        }
    }
};

playback_stream_capture_callback_sbs g_playback_stream_capture_callback_sbs;


class play_callback_sbs : public play_callback
{
public:
    virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused)
    {
        send_play = true;
        is_playing = true;
    }

    virtual void on_playback_edited(metadb_handle_ptr p_track) {}
    virtual void on_playback_new_track(metadb_handle_ptr p_track) {}

    virtual void on_playback_stop(play_control::t_stop_reason p_reason)
    {
        is_playing = false;
        slim_server->connection_manager_.send_stream_stop_all();
        http_server->connection_manager_.stop_all();
    }

    virtual void on_playback_seek(double) {}

    virtual void on_playback_pause(bool p_state)
    {
        // p_state is true when pausing, false when unpausing
        is_playing = !p_state;
        if (p_state)
        {
            slim_server->connection_manager_.send_stream_pause_all();
        }
        else
        {
            slim_server->connection_manager_.send_stream_unpause_all();
        }
    }

    virtual void on_playback_dynamic_info(const file_info &) {}
    virtual void on_playback_dynamic_info_track(const file_info &) {}
    virtual void on_playback_time(double) {}
    virtual void on_volume_change(float p_new_val) {}
};

play_callback_sbs g_play_callback_sbs;


class preferences_page_myimpl : public preferences_page_impl<CMyPreferences>
{
    // preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
public:
    const char * get_name()
    {
        return COMPONENT_NAME;
    }

    GUID get_guid()
    {
        // {0995A30B-A883-4365-8085-3CA946C7A6A3}
        static const GUID guid = { 0x0995A30B, 0xA883, 0x4365, { 0x80, 0x85, 0x3C, 0xA9, 0x46, 0xC7, 0xA6, 0xA3 } };
        return guid;
    }

    GUID get_parent_guid()
    {
        return guid_tools;
    }
};

static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_sbs_factory;


void g_http_server_thread_worker()
{
    http_server = new http::server::server("0.0.0.0", cfg_http_port.get_value(), app_path);
    http_server->run();
}

void g_slim_server_thread_worker()
{
    slim_server = new slim::server::server("0.0.0.0", cfg_slim_port.get_value(),
                                           app_path + "\\index.html");
    slim_server->run();
}

void g_start_server()
{
    http_server_thread = new boost::thread(g_http_server_thread_worker);
    slim_server_thread = new boost::thread(g_slim_server_thread_worker);
}

void g_stop_server()
{
    if (slim_server_thread != NULL)
    {
        slim_server->stop();
        slim_server_thread->join();
        slim_server_thread = NULL;
    }

    if (http_server_thread != NULL)
    {
        http_server->stop();
        http_server_thread->join();
        http_server_thread = NULL;
    }
}

int g_get_bps()
{
    return (cfg_bps == 1) ? 24 : 16;
}

void g_register_callback_sbs()
{
    static_api_ptr_t<playback_stream_capture>()->add_callback(&g_playback_stream_capture_callback_sbs);

    unsigned int play_flags =
        play_callback::flag_on_playback_starting |
        play_callback::flag_on_playback_stop |
        play_callback::flag_on_playback_pause;

    static_api_ptr_t<play_callback_manager>()->register_callback(&g_play_callback_sbs, play_flags, false);
}

void g_unregister_callback_sbs()
{
    static_api_ptr_t<playback_stream_capture>()->remove_callback(&g_playback_stream_capture_callback_sbs);
    static_api_ptr_t<play_callback_manager>()->unregister_callback(&g_play_callback_sbs);
}

void g_apply_preferences()
{
    g_stop_server();

    if (cfg_enable)
    {
        g_start_server();
        g_register_callback_sbs();
    }
    else
    {
        g_unregister_callback_sbs();
    }
}


DECLARE_COMPONENT_VERSION(COMPONENT_NAME, COMPONENT_VERSION, COMPONENT_NAME" v"COMPONENT_VERSION);
VALIDATE_COMPONENT_FILENAME("foo_sbs.dll");
