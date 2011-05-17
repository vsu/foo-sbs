#ifndef _FOO_SBS_H_
#define _FOO_SBS_H_

void g_http_server_thread_worker();
void g_slim_server_thread_worker();
void g_start_server();
void g_stop_server();
int g_get_bps();
void g_register_callback_sbs();
void g_unregister_callback_sbs();
void g_apply_preferences();

#endif