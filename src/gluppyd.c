#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <gconf/gconf-client.h>
#include <location/location-gps-device.h>
#include <location/location-gpsd-control.h>

#include "gconf-keys.h"
#include "latitude.h"

static GMainLoop* loop = NULL;
static CURL* curl = NULL;

static void
signal_handler(int sig)
{
	if(loop != NULL && g_main_loop_is_running(loop)) {
		g_main_loop_quit(loop);
	}
}

static void
gconf_key_change_callback(GConfClient* client, guint conn_id, GConfEntry* entry, gpointer userData)
{
    const GConfValue* value = NULL;
    const gchar* keyname = NULL;
    gchar* strValue = NULL;
    gchar* strName = NULL;
    
    g_print("gconf_key_change_callback.\n");
    keyname = gconf_entry_get_key(entry);
    value = gconf_entry_get_value(entry);
    
    strValue = gconf_value_to_string(value);
    
    g_print("Changed [%s=%s]\n", keyname, strValue);
    g_free(strValue);
}

static void
on_location_changed(LocationGPSDevice *device, gpointer data)
{
    if(!device)
        return;

    if(device->fix) {
        g_print("lat = %f, lng = %f\n",
            device->fix->latitude,
            device->fix->longitude);
    }    
}

int
main(int argc, char **argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);

	GConfClient* client = NULL;
	GError* error = NULL;
    LocationGPSDControl* control = NULL;
    LocationGPSDevice* device = NULL;
    
	curl = curl_easy_init();
    
	g_type_init();
	
    loop = g_main_loop_new(NULL, FALSE);
	
    // SETUP GCONF LISTENERS
	client = gconf_client_get_default();
    
    gconf_client_add_dir(client,
                         SERVICE_GCONF_ROOT,
                         GCONF_CLIENT_PRELOAD_NONE,
                         &error);
    if(error != NULL) g_error("error: %s\n", error->message);
    
    gconf_client_notify_add(client,
                            SERVICE_GCONF_ROOT,
                            gconf_key_change_callback, NULL, NULL, &error);
    if(error != NULL) g_error("error: %s\n", error->message);
	
    // SETUP GPS LISTENERS
    control = location_gpsd_control_get_default();
    device = g_object_new(LOCATION_TYPE_GPS_DEVICE, NULL);
    
    g_object_set(G_OBJECT(control),
                    "preferred-method", LOCATION_METHOD_USER_SELECTED,
                    "preferred-interval", LOCATION_INTERVAL_DEFAULT,
                    NULL);
    
    g_signal_connect(device, "changed", G_CALLBACK(on_location_changed), device);
    
    // BEGIN MAIN LOOP
    g_main_loop_run(loop);

    // CLEANUP
    curl_easy_cleanup(curl);
    g_object_unref(client);
    g_object_unref(device);
    g_object_unref(control);
	return EXIT_SUCCESS;
}
