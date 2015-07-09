/* Copyright (C) 2015 Timo Dritschler <timo.dritschler@kit.edu>
   (Karlsruhe Institute of Technology)

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 2.1 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
   details.

   You should have received a copy of the GNU Lesser General Public License along
   with this library; if not, write to the Free Software Foundation, Inc., 51
   Franklin St, Fifth Floor, Boston, MA 02110, USA
*/


#include <stdlib.h>
#include <string.h>
#include "uca-kiro-camera.h"
#include "uca/uca-plugin-manager.h"


typedef struct {
    gboolean exit_flag;
    UcaCamera *cam;
    KiroMessenger *messenger;
    gulong peer_rank;
    gulong *signal_handlers;
    GParamSpec **properties;
} KiroCsData;


static gulong
pspec_size (GType type)
{
    gulong ret = 0;

    switch (type) {
        case G_TYPE_BOOLEAN:
            ret = sizeof (GParamSpecBoolean);
            break;
        case G_TYPE_CHAR:
            ret = sizeof (GParamSpecChar);
            break;
        case G_TYPE_INT:
            ret = sizeof (GParamSpecInt);
            break;
        case G_TYPE_UINT:
            ret = sizeof (GParamSpecUInt);
            break;
        case G_TYPE_LONG:
            ret = sizeof (GParamSpecLong);
            break;
        case G_TYPE_ULONG:
            ret = sizeof (GParamSpecULong);
            break;
        case G_TYPE_INT64:
            ret = sizeof (GParamSpecInt64);
            break;
        case G_TYPE_UINT64:
            ret = sizeof (GParamSpecUInt64);
            break;
        case G_TYPE_FLOAT:
            ret = sizeof (GParamSpecFloat);
            break;
        case G_TYPE_DOUBLE:
            ret = sizeof (GParamSpecDouble);
            break;
        default:
            ret = sizeof (GParamSpec);    
    }

    return ret;
}

static gulong
gtype_size (GType type)
{
    gulong ret = 0;

    switch (type) {
        case G_TYPE_BOOLEAN:
            ret = sizeof (gboolean);
            break;
        case G_TYPE_CHAR:
            ret = sizeof (gchar);
            break;
        case G_TYPE_INT:
            ret = sizeof (gint);
            break;
        case G_TYPE_ENUM:
            ret = sizeof (gint);
            break;
        case G_TYPE_UINT:
            ret = sizeof (guint);
            break;
        case G_TYPE_LONG:
            ret = sizeof (glong);
            break;
        case G_TYPE_ULONG:
            ret = sizeof (gulong);
            break;
        case G_TYPE_INT64:
            ret = sizeof (gint64);
            break;
        case G_TYPE_UINT64:
            ret = sizeof (guint64);
            break;
        case G_TYPE_FLOAT:
            ret = sizeof (gfloat);
            break;
        case G_TYPE_DOUBLE:
            ret = sizeof (gdouble);
            break;
        default:
            //ERROR
            break;
    }

    return ret;
}


static void
print_cam_name (gchar *name, gpointer unused)
{
    (void) unused;
    g_print ("-- %s\n", name);
}


static void
peer_inform_update (UcaCamera *cam, GParamSpec *pspec, KiroCsData *data)
{
    g_print ("Updated %s.\n", pspec->name); 

    GError *error = NULL;

    GVariant *tmp = read_property_scalar (G_OBJECT (cam), pspec->name, pspec->value_type);
    gsize data_size = g_variant_get_size (tmp);

    PropUpdate *test = g_malloc0 (sizeof (PropUpdate) + data_size);
    test->id = property_id_from_name (pspec->name);
    test->type[0] = gtype_to_gvariant_class (pspec->value_type);
    test->size = data_size;
    g_variant_store (tmp, test->val);
    g_variant_unref (tmp);

    KiroMessage message;
    message.peer_rank = data->peer_rank;
    message.msg = KIROCS_UPDATE;
    message.payload = test;
    message.size = sizeof (PropUpdate) + data_size;

    kiro_messenger_send_blocking (data->messenger, &message, &error);
    if (error) {
        g_free (test);
        g_error ("Oh shit! (%s)", error->message);
    }

    g_free (test);
}


static gulong
setup_signal_handler (UcaCamera *cam, GParamSpec *pspec, KiroCsData *data)
{
    GString *signal_name = g_string_new ("notify::");
    signal_name = g_string_append (signal_name, pspec->name);
    gulong ret = g_signal_connect (cam, signal_name->str, G_CALLBACK (peer_inform_update), data);
    g_string_free (signal_name, TRUE);
    return ret;
}


static void
null_callback (gpointer unused)
{
    (void)unused;
}


static inline gpointer
unpack_scalar (PropUpdate *src)
{
    //Alloc max scalar size
    gpointer out = g_malloc0 (sizeof (guint64));

    GVariant *var = g_variant_new_from_data ((const GVariantType *)src->type, src->val, \
            src->size, TRUE, null_callback, NULL);
    g_variant_get (var, src->type, out);
    g_variant_unref (var);
    return out;
}     

static KiroContinueFlag
connect_callback (gulong rank, gulong *storage)
{
    *storage = rank;
    return KIRO_CALLBACK_REMOVE;
}


// MAIN HANDLER //
static KiroContinueFlag
receive_callback (KiroMessageStatus *status, KiroCsData *data)
{

    if (status->message->msg == KIROCS_EXIT) {
        g_message ("Peer requested shut down...");
        data->exit_flag = TRUE;
    }

    if (status->message->msg == KIROCS_UPDATE) {
        PropUpdate *update = (PropUpdate *)status->message->payload; 

        g_debug ("Unpacking ID %u\n", update->id);
        gpointer unpacked = unpack_scalar (update); 

        //Don't forget the -1, because the index starts at 0, but property IDs
        //start at 1...
        update_property_scalar (G_OBJECT (data->cam),
                                data->properties[update->id -1]->name,
                                data->properties[update->id -1]->value_type,
                                data->signal_handlers[update->id],
                                unpacked);

        g_free (unpacked);

    }

    status->request_cleanup = TRUE;

    return KIRO_CALLBACK_CONTINUE;
}


int main (int argc, char *argv[])
{

    GOptionContext *context;
    GError *error = NULL;

    static gchar *camera_name = "mock";
    static gchar *addr = "127.0.0.1";
    static gchar *port = "60010";
    static gboolean list = FALSE;

    static GOptionEntry entries[] = {
        { "camera", 'c', 0, G_OPTION_ARG_STRING, &camera_name, "Uca camera plugin to load", NULL },
        { "address", 'a', 0, G_OPTION_ARG_STRING, &addr, "Address to listen on", NULL },
        { "port", 'p', 0, G_OPTION_ARG_STRING, &port, "Port to listen on", NULL },
        { "list", 'l', 0, G_OPTION_ARG_NONE, &list, "List all available plugins and exit", NULL },
        { NULL }
    };

#if !(GLIB_CHECK_VERSION (2, 36, 0))
    g_type_init ();
#endif

    context = g_option_context_new ("-l | [-c <CAMERA PLUGIN>] [-a <ADDRESS>] [-p <PORT>]");
    g_option_context_set_summary (context, "kiro-camera-server provides a remote control host for libuca cameras.\n\
Once the server is started, you can use the uca-kiro-camera plugin to connect to the server\n\
over an InfiniBand network and control the loaded remote camera as if it was connected locally.");
    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Error parsing options: %s\n", error->message);
        exit (-1);
    }

    if (argc >= 2) {
        g_print ("%s", g_option_context_get_help (context, TRUE, NULL));
        exit (0);
    }

    KiroCsData data;
    data.exit_flag = FALSE;
    data.peer_rank = 0;

    UcaPluginManager *pm = uca_plugin_manager_new ();
    if (list) {
        GList *names = uca_plugin_manager_get_available_cameras (pm);
        if (!names) {
            g_print ("No available plugins found.\n");
        }
        else {
            g_print ("The following Uca camera plugins are available:\n");
            g_list_foreach (names, (GFunc) print_cam_name, NULL);
            g_list_free_full (names, g_free);
        }
        g_object_unref (pm);
        exit (0);
    }

    data.cam = uca_plugin_manager_get_camera (pm, camera_name, &error, NULL);
    if (!(data.cam)) {
        g_print ("Failed to load plugin '%s': %s. Exiting.\n", camera_name, error->message);
        g_object_unref (pm);
        exit (-1);
    }
    g_object_unref (pm);


    data.messenger = kiro_messenger_new ();
    kiro_messenger_start_listen (data.messenger, addr, port, (KiroConnectCallbackFunc) connect_callback, &(data.peer_rank), &error);
    if (error) {
        g_print ("Failed to launch Kiro Server: %s Exiting.\n", error->message);
        g_object_unref (data.cam);
        exit (-1);
    }

    // wait for the first peer
    while (data.peer_rank == 0) {};

    guint num_properties = 0;
    data.signal_handlers = NULL;
    data.properties = g_object_class_list_properties (G_OBJECT_GET_CLASS (data.cam), &num_properties);
    if (!(*data.properties)) {
        g_object_unref (data.messenger);
        g_object_unref (data.cam);
        g_error ("No properties exposed by camera '%s'.", camera_name);
    }
    else {
        data.signal_handlers = g_malloc0 (sizeof (gulong) * (num_properties + 1)); //0 is not a valid property ID

        for (guint idx=0; idx < num_properties; idx++) {

            gint prop_id = property_id_from_name (data.properties[idx]->name);
            if (0 >= prop_id) {
                //NONE Base Property.
                //Inform peer and setup
                continue;
            }

            data.signal_handlers[prop_id] = setup_signal_handler (data.cam, data.properties[idx], &data);
        }
    }

    //All done. Send READY
    KiroMessage message;
    message.peer_rank = data.peer_rank;
    message.msg = KIROCS_READY;
    message.size = 0;
    message.payload = NULL;

    if (!kiro_messenger_send_blocking (data.messenger, &message, &error)) {
        g_error ("Oh shit!");
        //TODO
        //Things
    }


    kiro_messenger_add_receive_callback (data.messenger, (KiroMessageCallbackFunc)receive_callback, &data);
    while (!(data.exit_flag)) {};


    g_free (data.properties);
    g_free (data.signal_handlers);
    kiro_messenger_free (data.messenger);
    g_object_unref (data.cam);
    return 0;
};


