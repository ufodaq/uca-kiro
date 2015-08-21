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
#include <unistd.h>
#include "uca-kiro-camera.h"
#include "uca/uca-plugin-manager.h"


typedef struct {
    gboolean exit_flag;
    UcaCamera *cam;
    KiroMessenger *messenger;
    gulong peer_rank;
    gulong *signal_handlers;
    GParamSpec **properties;
    guint n_properties;
    KiroRequest *peer_update_request;
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
send_clear_callback (KiroRequest *request, gpointer data)
{
    (void) data;
    if (request->status != KIRO_MESSAGE_SEND_SUCCESS)
        g_error ("OH SHIT!");
    else
        g_debug ("Peer informed successfully");

    if (request->message->payload)
        g_free (request->message->payload);
    g_free (request->message);
    g_free (request);
}


static void
peer_inform_update (UcaCamera *cam, GParamSpec *pspec, KiroCsData *data)
{
    g_print ("Updated %s.\n", pspec->name);

    gpointer buff = g_malloc0 (sizeof (guint64));
    g_object_get (G_OBJECT (cam),
                  pspec->name, buff,
                  NULL);

    KiroMessage *message = g_malloc0 (sizeof (KiroMessage));
    message->msg = KIROCS_UPDATE;
    message->size = sizeof (PropUpdateScalar);
    message->payload = g_malloc0 (message->size);

    PropUpdateScalar *update = message->payload;
    update->base.id = property_id_from_name (pspec->name, data->n_properties, data->properties);
    update->base.size = 1;
    update->base.scalar = TRUE;

    memcpy (&update->prop_raw, buff, sizeof (guint64));

    GError *error = NULL;
    kiro_messenger_send_blocking (data->messenger, message, data->peer_rank, &error);
    if (error) {
        g_free (message);
        g_error ("Oh shit! (%s)", error->message);
        g_error_free (error);
    }
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


static KiroContinueFlag
connect_callback (gulong rank, gulong *storage)
{
    *storage = rank;
    return KIRO_CALLBACK_REMOVE;
}


// MAIN HANDLER //
static void
receive_callback (KiroRequest *request, KiroCsData *data)
{

    if (request->message->msg == KIROCS_EXIT) {
        g_message ("Peer requested shut down...");
        data->exit_flag = TRUE;
    }

    if (request->message->msg == KIROCS_UPDATE) {
        PropUpdate *update = (PropUpdate *)request->message->payload;

        g_signal_handler_block (data->cam, data->signal_handlers[update->id]);
        //Don't forget the -1, because the index starts at 0, but property IDs
        //start at 1...
        const gchar *name = data->properties[update->id -1]->name;
        g_debug ("Unpacking '%s' with ID %u\n", name, update->id);

        if (update->scalar == TRUE) {
            PropUpdateScalar *scalar_update = (PropUpdateScalar *)update;
            GValue tmp = G_VALUE_INIT;
            g_value_init (&tmp, data->properties[update->id -1]->value_type);
            g_value_set_from_raw_data (&tmp, &scalar_update->prop_raw);
            g_object_set_property (G_OBJECT (data->cam), name, &tmp);
        }

        g_signal_handler_unblock (data->cam, data->signal_handlers[update->id]);
        g_debug ("Done.");
    }

    if (request->message->msg == KIROCS_FETCH) {
        gchar* property_name = (gchar *)request->message->payload;

        KiroRequest *request = g_malloc0 (sizeof (KiroRequest));
        request->peer_rank = data->peer_rank;
        request->callback = send_clear_callback;
        request->user_data = NULL;
        request->message = g_malloc0 (sizeof (KiroMessage));

        request->message->msg = KIROCS_UPDATE;
        request->message->size = sizeof (PropUpdateScalar);
        request->message->payload = g_malloc0 (request->message->size);
        PropUpdateScalar *update = (PropUpdateScalar *)request->message->payload;

        g_object_get (data->cam, property_name, &update->prop_raw, NULL);

        update->base.id = property_id_from_name (property_name, data->n_properties, data->properties);
        update->base.size = 1;
        update->base.scalar = TRUE;

        GError *error = NULL;
        if (!kiro_messenger_send (data->messenger, request, &error)) {
            g_error ("Oh shit! '%s'", error->message);
            g_error_free (error);
            //TODO
            //Things
        }
    }

    if (request->message->payload)
        g_free (request->message->payload);
    g_free (request->message);

    kiro_messenger_receive (data->messenger, request);
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


    KiroMessage message;
    data.signal_handlers = NULL;
    data.properties = g_object_class_list_properties (G_OBJECT_GET_CLASS (data.cam), &data.n_properties);
    if (!(*data.properties)) {
        g_object_unref (data.messenger);
        g_object_unref (data.cam);
        g_error ("No properties exposed by camera '%s'.", camera_name);
    }
    else {
        data.signal_handlers = g_malloc0 (sizeof (gulong) * (data.n_properties + 1)); //0 is not a valid property ID

        for (guint idx=0; idx < data.n_properties; idx++) {

            if (idx >= N_BASE_PROPERTIES - 1) {
                //NONE Base Property.
                //Inform peer and setup
                g_debug ("Asking peer to install property '%s' with ID %u", data.properties[idx]->name, idx + 1);
                message.msg = KIROCS_INSTALL;
                message.size = sizeof (PropertyRequisition) + strlen (data.properties[idx]->name);
                message.payload = g_malloc0 (message.size);

                PropertyRequisition *req = message.payload;
                req->id = idx + 1;
                req->value_type = data.properties[idx]->value_type;

                switch (req->value_type) {
                    case G_TYPE_BOOLEAN:
                        req->spec.bool_spec = *(GParamSpecBoolean *)data.properties[idx];
                        break;
                    default:
                        g_debug ("FOOBAR");
                }
                strcpy (req->name, data.properties[idx]->name);

                if (!kiro_messenger_send_blocking (data.messenger, &message, data.peer_rank, &error)) {
                    g_error ("Oh shit!");
                    //TODO
                    //Things
                }
                g_free (req);
            }
            else
                peer_inform_update (data.cam, data.properties[idx], &data);

            guint handler_id = setup_signal_handler (data.cam, data.properties[idx], &data);
            data.signal_handlers[idx + 1] = handler_id;
            g_debug ("Setup handler for property '%s' with handler ID %u", data.properties[idx]->name, handler_id);
        }
    }

    //All done. Send READY
    message.msg = KIROCS_READY;
    message.size = 0;
    message.payload = NULL;

    if (!kiro_messenger_send_blocking (data.messenger, &message, data.peer_rank, &error)) {
        g_error ("Oh shit!");
        //TODO
        //Things
    }

    KiroRequest request;
    request.id = 0;
    request.callback = (KiroMessageCallbackFunc) receive_callback;
    request.user_data = (gpointer) &data;

    kiro_messenger_receive (data.messenger, &request);
    while (!(data.exit_flag)) {
        sleep (5);
        //TODO
        //Check if connection is still alive, and if not, reinitialize
    };


    g_free (data.properties);
    g_free (data.signal_handlers);
    kiro_messenger_free (data.messenger);
    g_object_unref (data.cam);
    return 0;
};


