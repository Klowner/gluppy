#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gconf/gconf-client.h>
#include <location/location-gps-device.h>
#include <location/location-gpsd-control.h>

#include "gconf-keys.h"
#include "latitude.h"

static GMainLoop* loop = NULL;
static CURL* curl = NULL;
static int update_timer_id = 0;
static LocationGPSDControl* control = NULL;
static LocationGPSDevice* device = NULL;

struct {
    float lat;
    float lng;
    int acc;
} last_fix;

struct GluppyPrefs {
    gchar *username;
    gchar *password;
    gint update_interval_s;
    gboolean authenticated;
    gboolean run;
};

static void
signal_handler(int sig)
{
	if(loop != NULL && g_main_loop_is_running(loop)) {
		g_main_loop_quit(loop);
	}
}

static void
on_location_changed(LocationGPSDevice *device, gpointer data)
{
    if(!device)
        return;

    if(device->fix) {
        last_fix.lat = device->fix->latitude;
        last_fix.lng = device->fix->longitude;
        last_fix.acc = device->fix->eph / 100; // Horizontal accuracy (cm) to meters 
        /*
        g_print("lat = %f, lng = %f, acc = %e\n",
            device->fix->latitude,
            device->fix->longitude,
            device->fix->eph);
        */
    }    
}

static gboolean
on_update_location(gpointer data) {
    struct GluppyPrefs* prefs = (struct GluppyPrefs*)data;

    if(!prefs->authenticated && prefs->username && prefs->password) {
        prefs->authenticated = !latitude_authenticate(curl, prefs->username, prefs->password);
        if(!prefs->authenticated) g_error("Auth error.\n");
    } 
 
    if(prefs->authenticated && last_fix.lat && last_fix.lng && last_fix.acc) {
        g_print("Updating location [ lat = %f, lng = %f, acc = %d ]\n", last_fix.lat, last_fix.lng, last_fix.acc);
        latitude_set_location(curl, &last_fix.acc, &last_fix.lat, &last_fix.lng);      
    }
    
    return TRUE;
}

static void
start_updating(LocationGPSDControl *control, struct GluppyPrefs *prefs) {
    location_gpsd_control_start(control);
    
    if(update_timer_id) {
        g_source_remove(update_timer_id);
        update_timer_id = 0;
    }
    update_timer_id = g_timeout_add(prefs->update_interval_s * 1000, on_update_location, prefs);
}

static void
stop_updating(LocationGPSDControl *control) {
    location_gpsd_control_stop(control);

    if(update_timer_id) {
        g_source_remove(update_timer_id);
        update_timer_id = 0;
    }
}

static void
gconf_key_change_callback(GConfClient* client, guint conn_id, GConfEntry* entry, gpointer userData)
{
    struct GluppyPrefs *prefs = (struct GluppyPrefs*)userData;
    const GConfValue* value = NULL;
    const gchar* keyname = NULL;
    //gchar* strValue = NULL;
    //gchar* strName = NULL;
    
    keyname = gconf_entry_get_key(entry);
    value = gconf_entry_get_value(entry);
    
    stop_updating(control);
    
    if(!strcmp(keyname, SERVICE_KEY_USER)) {
        if(prefs->username)
            g_free(prefs->username);
        prefs->username = gconf_value_to_string(value);
        prefs->authenticated = FALSE;
        
    } else if(!strcmp(keyname, SERVICE_KEY_PASS)) {
        if(prefs->password)
            g_free(prefs->password);
        prefs->password = gconf_value_to_string(value);
        prefs->authenticated = FALSE;
        
    } else if(!strcmp(keyname, SERVICE_KEY_UPDATE_INTERVAL)) {
        prefs->update_interval_s = gconf_value_get_int(value);
        //g_print("update interval changed to: %d %d\n", prefs->update_interval_s*1000, update_timer_id);
        
    } else if(!strcmp(keyname, SERVICE_KEY_RUN)) {
        prefs->run = gconf_value_get_bool(value);      
    }
    
    if(prefs->run) 
        start_updating(control, prefs);
}

static void
load_prefs(GConfClient *client, struct GluppyPrefs *prefs) {
    prefs->username = gconf_client_get_string(client, SERVICE_KEY_USER, NULL);
    prefs->password = gconf_client_get_string(client, SERVICE_KEY_PASS, NULL);
    prefs->update_interval_s = gconf_client_get_int(client, SERVICE_KEY_UPDATE_INTERVAL, NULL);
    prefs->authenticated = FALSE;
    prefs->run = gconf_client_get_bool(client, SERVICE_KEY_RUN, NULL);
}

int
main(int argc, char **argv)
{
	struct GluppyPrefs prefs;
    prefs.authenticated = FALSE;
    
    signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);

	GConfClient* client = NULL;
	GError* error = NULL;
	
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/home/user/.gluppycookies");

	g_type_init();
	
    loop = g_main_loop_new(NULL, FALSE);
	
    // SETUP GCONF LISTENERS
	client = gconf_client_get_default();
    
    load_prefs(client, &prefs);
   
    gconf_client_add_dir(client,
                         SERVICE_GCONF_ROOT,
                         GCONF_CLIENT_PRELOAD_NONE,
                         &error);
    if(error != NULL) g_error("error: %s\n", error->message);
    
    gconf_client_notify_add(client,
                            SERVICE_GCONF_ROOT,
                            gconf_key_change_callback, &prefs, NULL, &error);
    if(error != NULL) g_error("error: %s\n", error->message);
	
    // SETUP GPS LISTENERS
    control = location_gpsd_control_get_default();
    device = g_object_new(LOCATION_TYPE_GPS_DEVICE, NULL);
    
    g_object_set(G_OBJECT(control),
                    "preferred-method", LOCATION_METHOD_GNSS | LOCATION_METHOD_AGNSS,
                    "preferred-interval", LOCATION_INTERVAL_120S,
                    NULL);
    
    g_signal_connect(device, "changed", G_CALLBACK(on_location_changed), device);
        
    // IF USERNAME AND PASSWORD ARE SET, THEN
    // WE NEED TO AUTHENTICATE AND BEGIN GPS UPDATING
    if(prefs.username && prefs.password && prefs.run ) {
        start_updating(control, &prefs);
    } 
 
    // BEGIN MAIN LOOP
    g_main_loop_run(loop);

    // CLEANUP
    stop_updating(control);
    
    //location_gpsd_control_stop(control);
    curl_easy_cleanup(curl);
    g_object_unref(client);
    g_object_unref(device);
    g_object_unref(control);
	
    g_free(prefs.username);
    g_free(prefs.password);
    
    return EXIT_SUCCESS;
}
