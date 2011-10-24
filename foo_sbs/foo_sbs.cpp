/*
 * (c) 2011 Victor Su
 *
 * This program is open source. For license terms, see the LICENSE file.
 *
 */
#include <winsock2.h>
#include <foobar2000.h>
#include "../ATLHelpers/ATLHelpers.h"
#include <string>
#include <sstream>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include "foo_sbs.h"
#include "defs.h"
#include "shm.h"
#include "preferences.h"
#include "../http_server/server.hpp"
#include "../slim_server/server.hpp"

#define COMPONENT_NAME		"Squeezebox Server"
#define COMPONENT_VERSION	"0.1"

http::server::server *http_server;
boost::thread *http_server_thread = NULL;

slim::server::server *slim_server;
boost::thread *slim_server_thread = NULL;

bool send_play = false;
bool is_playing = false;
bool callback_registered = false;


class initquit_sbs : public initquit
{
    void on_init()
    {
        // Shared memory front-end that is able to construct objects
        // associated with a c-string. Erase previous shared memory with the name
        // to be used and create the memory segment at the specified address and initialize resources
        shared_memory_object::remove(FOO_SBS_SHM_SEGMENT);
        managed_shared_memory segment(
            create_only, 
            FOO_SBS_SHM_SEGMENT,  // segment name
            65536);               // segment size in bytes

        // Initialize the shared memory STL-compatible allocator
        ShMemAllocator alloc_inst (segment.get_segment_manager());

        // Construct a shared memory map.
        // Note that the first parameter is the comparison function,
        // and the second one the allocator.
        // This the same signature as std::map's constructor taking an allocator
        ShMemMap *shm_map = 
            segment.construct<ShMemMap>(FOO_SBS_SHM_MAP)           // object name
                                       (std::less<std::string>(),  // first ctor parameter
                                        alloc_inst);               // second ctor parameter

        if (cfg_enable.get_value() != 0)
        {
            g_start_server();
            g_register_callback_sbs();
        }
    }

    void on_quit()
    {
        g_stop_server();
        g_unregister_callback_sbs();
        shared_memory_object::remove(FOO_SBS_SHM_SEGMENT);
    }
};

static initquit_factory_t<initquit_sbs> g_initquit_sbs_factory;


class playback_stream_capture_callback_sbs : public playback_stream_capture_callback
{
public:
    void on_chunk(const audio_chunk & chunk)
    {
        unsigned int srate = chunk.get_srate();

        if (send_play)
        {
            send_play = false;

            slim_server->get_connection_manager().send_stream_play_all(
                URL_PATH_STREAM, cfg_http_port.get_value(), g_get_bps(), srate);
        }

        if (is_playing)
        {
            mem_block_container_impl_t<> data;
            chunk.to_raw_data(data, g_get_bps());
            
            std::set<http::server::connection_ptr> connections = 
                http_server->get_connection_manager().get_connections();
            
            std::set<http::server::connection_ptr>::const_iterator c;
            for (c = connections.begin(); c != connections.end(); ++c)
            {
                http::server::connection_ptr c_ptr = (http::server::connection_ptr)(*c);

                // if sending data to the http stream connection returns with 
                // an error, close the connection and call slim server to send
                // a new play command to the device.
                if (c_ptr->send_data(data.get_ptr(), data.get_size()) < 0)
                {
                    std::string mac_address = c_ptr->get_mac_address();

                    http_server->get_connection_manager().stop(c_ptr);

                    if (!mac_address.empty())
                    {
                        slim_server->get_connection_manager().send_stream_play(
                            mac_address, URL_PATH_STREAM, cfg_http_port.get_value(), g_get_bps(), srate);
                    }
                }
            }
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
        slim_server->get_connection_manager().send_stream_stop_all();
        http_server->get_connection_manager().stop_all();
    }

    virtual void on_playback_seek(double) {}

    virtual void on_playback_pause(bool p_state)
    {
        // p_state is true when pausing, false when unpausing
        is_playing = !p_state;
        if (p_state)
        {
            slim_server->get_connection_manager().send_stream_pause_all();
        }
        else
        {
            slim_server->get_connection_manager().send_stream_unpause_all();
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
    http_server = new http::server::server("0.0.0.0", cfg_http_port.get_value(), "");
    http_server->run();
    delete http_server;
}

void g_slim_server_thread_worker()
{
    slim_server = new slim::server::server("0.0.0.0", cfg_slim_port.get_value());
    slim_server->run();
    delete slim_server;
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
        delete slim_server_thread;
        slim_server_thread = NULL;
    }

    if (http_server_thread != NULL)
    {
        http_server->stop();
        http_server_thread->join();
        delete http_server_thread;
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

    callback_registered = true;
}

void g_unregister_callback_sbs()
{
    if (callback_registered)
    {
        static_api_ptr_t<playback_stream_capture>()->remove_callback(&g_playback_stream_capture_callback_sbs);
        static_api_ptr_t<play_callback_manager>()->unregister_callback(&g_play_callback_sbs);
        
        callback_registered = false;
    }
}

void g_apply_preferences()
{
    g_stop_server();

    if (cfg_enable.get_value() != 0)
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
