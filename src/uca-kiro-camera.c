/* Copyright (C) 2011, 2012 Matthias Vogelgesang <matthias.vogelgesang@kit.edu>
   (Karlsruhe Institute of Technology)

   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 2.1 of the License, or (at your
   option) any later version.

   This library is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
   details.

   You should have received a copy of the GNU Lesser General Public License along
   with this library; if not, write to the Free Software Foundation, Inc., 51
   Franklin St, Fifth Floor, Boston, MA 02110, USA */


#include <gmodule.h>
#include <gio/gio.h>
#include <string.h>
#include <math.h>
#include <kiro/kiro-messenger.h>
#include "uca-kiro-camera.h"


#define UCA_KIRO_CAMERA_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), UCA_TYPE_KIRO_CAMERA, UcaKiroCameraPrivate))

static void uca_kiro_initable_iface_init (GInitableIface *iface);
GError *initable_iface_error = NULL;

G_DEFINE_TYPE_WITH_CODE (UcaKiroCamera, uca_kiro_camera, UCA_TYPE_CAMERA,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                uca_kiro_initable_iface_init))


/**
 * UcaCameraError:
   @UCA_KIRO_CAMERA_ERROR_MISSING_ADDRESS: No KIRO address ('kiro://<IP>:<PORT>') property was supplied during camera creation
   @UCA_KIRO_CAMERA_ERROR_ADDRESS_WRONG_FORMAT: KIRO address has the wrong format. Expected: 'kiro://<IP>:<PORT>'
   @UCA_KIRO_CAMERA_ERROR_KIRO_CONNECTION_FAILED: Failed to establish a KIRO connection to the given TANGO server
 */
GQuark uca_kiro_camera_error_quark()
{
    return g_quark_from_static_string ("uca-kiro-camera-error-quark");
}


enum {
    PROP_KIRO_ADDRESS = N_BASE_PROPERTIES,
    PROP_KIRO_REMOTE_NAME,
    N_PROPERTIES
};

static const gint kiro_overrideables[] = {
    PROP_NAME,
    0,
};

static GParamSpec *kiro_properties[N_PROPERTIES] = { NULL, };

struct _UcaKiroCameraPrivate {
    guint8 *dummy_data;
    guint current_frame;
    gchar *kiro_address_string;
    gchar *kiro_address;
    gchar *kiro_port;
    gchar *remote_name;
    GParamSpec **kiro_dynamic_attributes;

    gboolean thread_running;
    gboolean kiro_connected;
    gboolean construction_error;

    GThread *grab_thread;
    KiroMessenger *messenger;
    gulong peer_rank;

    guint roi_height;
    guint roi_width;
    guint bytes_per_pixel;
};

static gpointer
kiro_grab_func(gpointer data)
{
    /* UcaKiroCamera *kiro_camera = UCA_KIRO_CAMERA (data); */
    /* g_return_val_if_fail (UCA_IS_KIRO_CAMERA (kiro_camera), NULL); */

    /* UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (kiro_camera); */
    /* UcaCamera *camera = UCA_CAMERA (kiro_camera); */
    /* gdouble fps; */
    /* g_object_get (G_OBJECT (data), "frames-per-second", &fps, NULL); */
    /* const gulong sleep_time = (gulong) G_USEC_PER_SEC / fps; */

    /* while (priv->thread_running) { */
    /*     camera->grab_func (NULL, camera->user_data); */
    /*     g_usleep (sleep_time); */
    /* } */

    return NULL;
}

static void
uca_kiro_camera_start_recording(UcaCamera *camera, GError **error)
{
    /* gboolean transfer_async = FALSE; */
    /* UcaKiroCameraPrivate *priv; */
    /* g_return_if_fail(UCA_IS_KIRO_CAMERA (camera)); */

    /* priv = UCA_KIRO_CAMERA_GET_PRIVATE (camera); */
    /* g_object_get (G_OBJECT(camera), */
    /*         "transfer-asynchronously", &transfer_async, */
    /*         NULL); */

    /* //'Cache' ROI settings from TANGO World */
    /* g_object_get (G_OBJECT(camera), */
    /*         "roi-width", &priv->roi_width, */
    /*         NULL); */
    /* g_object_get (G_OBJECT(camera), */
    /*         "roi-height", &priv->roi_height, */
    /*         NULL); */

    /* size_t bits = 0; */
    /* g_object_get (G_OBJECT(camera), */
    /*         "sensor-bitdepth", &bits, */
    /*         NULL); */

    /* priv->bytes_per_pixel = 1; */
    /* if (bits > 8) priv->bytes_per_pixel++; */
    /* if (bits > 16) priv->bytes_per_pixel++; */
    /* if (bits > 24) priv->bytes_per_pixel++; */

    /* Tango::DevState state; */
    /* g_object_get (G_OBJECT(camera), */
    /*         "State", &state, */
    /*         NULL); */
    /* try { */
    /*     if (Tango::DevState::STANDBY == state) */
    /*         priv->tango_device->command_inout ("StartRecording"); */
    /* } */
    /* catch (Tango::DevFailed &e) { */
    /*     g_warning ("Failed to execute 'StartRecording' on the remote camera due to a TANGO exception.\n"); */
    /*     g_set_error (error, UCA_KIRO_CAMERA_ERROR, UCA_KIRO_CAMERA_ERROR_TANGO_EXCEPTION_OCCURED, */
    /*                  "A TANGO exception was raised: '%s'", (const char *)e.errors[0].desc); */
    /*     return; */
    /* } */


    /* /* */
    /*  * In case asynchronous transfer is requested, we start a new thread that */
    /*  * invokes the grab callback, otherwise nothing will be done here. */
    /*  *1/ */
    /* if (transfer_async) { */
    /*     GError *tmp_error = NULL; */
    /*     priv->thread_running = TRUE; */
    /*     priv->grab_thread = g_thread_create (kiro_grab_func, camera, TRUE, &tmp_error); */

    /*     if (tmp_error != NULL) { */
    /*         priv->thread_running = FALSE; */
    /*         g_propagate_error (error, tmp_error); */
    /*         try { */
    /*             priv->tango_device->command_inout ("StopRecording"); */
    /*         } */
    /*         catch (Tango::DevFailed &e) { */
    /*             g_warning ("Failed to execute 'StopRecording' on the remote camera due to a TANGO exception: '%s'\n", (const char *)e.errors[0].desc); */
    /*         } */
    /*     } */
    /* } */

    /* kiro_sb_thaw (priv->receive_buffer); */
}

static void
uca_kiro_camera_stop_recording(UcaCamera *camera, GError **error)
{
    /* g_return_if_fail(UCA_IS_KIRO_CAMERA (camera)); */
    /* UcaKiroCameraPrivate *priv; */
    /* priv = UCA_KIRO_CAMERA_GET_PRIVATE (camera); */

    /* Tango::DevState state; */
    /* g_object_get (G_OBJECT(camera), */
    /*         "State", &state, */
    /*         NULL); */

    /* try { */
    /*     if (Tango::DevState::RUNNING == state) */
    /*         priv->tango_device->command_inout ("StopRecording"); */
    /* } */
    /* catch (Tango::DevFailed &e) { */
    /*     g_warning ("Failed to execute 'StopRecording' on the remote camera due to a TANGO exception.\n"); */
    /*     g_set_error (error, UCA_KIRO_CAMERA_ERROR, UCA_KIRO_CAMERA_ERROR_TANGO_EXCEPTION_OCCURED, */
    /*                  "A TANGO exception was raised: '%s'", (const char *)e.errors[0].desc); */
    /* } */

    /* gboolean transfer_async = FALSE; */
    /* g_object_get(G_OBJECT (camera), */
    /*         "transfer-asynchronously", &transfer_async, */
    /*         NULL); */

    /* if (transfer_async) { */
    /*     priv->thread_running = FALSE; */
    /*     g_thread_join (priv->grab_thread); */
    /* } */

    /* kiro_sb_freeze (priv->receive_buffer); */
    /* g_free (priv->dummy_data); */
}

static void
uca_kiro_camera_trigger (UcaCamera *camera, GError **error)
{
}

static gboolean
uca_kiro_camera_grab (UcaCamera *camera, gpointer data, GError **error)
{
    /* g_return_val_if_fail (UCA_IS_KIRO_CAMERA (camera), FALSE); */

    /* UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (camera); */

    /* //This is a hack to make sure we actually wait for a new frame; */
    /* gpointer frame = kiro_sb_get_data_blocking (priv->receive_buffer); */

    /* kiro_sb_freeze (priv->receive_buffer); */
    /* //Element 0 might still be in the process of being written. */ 
    /* //Therefore, we take Element 1, to be sure this one is finished. */
    /* if (data) */
    /*     g_memmove (data, frame, priv->roi_width * priv->roi_height * priv->bytes_per_pixel); */
    /* kiro_sb_thaw (priv->receive_buffer); */

    return TRUE;
}


static gboolean
kiro_address_decode (const gchar *addr_in, gchar **addr, gchar **port, GError **error)
{
    if (!g_str_has_prefix (addr_in, "kiro://")) {
        g_set_error_literal (error, UCA_KIRO_CAMERA_ERROR, UCA_KIRO_CAMERA_ERROR_ADDRESS_WRONG_FORMAT,
                             "Address does not use 'kiro://' scheme.");
        return FALSE;
    }

    /* Pitfall: kiro will silently accept hostnames like kiro://localhost:5555
     * but not bind to it as it treats it like an interface name (like eth0).
     * We have to use IP addresses instead of DNS names.
     */
    gchar *host = g_strdup (&addr_in[7]);

    if (!g_ascii_isdigit (host[0]) && host[0] != '*')
        g_debug ("Treating address %s as interface device name. Use IP address if supplying a host was intended.", host);

    gchar **split = g_strsplit (host, ":", 2);

    if (!g_ascii_isdigit (*split[1])) {
            g_set_error (error, UCA_KIRO_CAMERA_ERROR, UCA_KIRO_CAMERA_ERROR_ADDRESS_WRONG_FORMAT,
                         "Address '%s' has wrong format", addr_in);
            g_strfreev (split);
            g_free (host);
            return FALSE;
    }

    *addr = g_strdup (split[0]);
    *port = g_strdup (split[1]);

    g_strfreev (split);
    g_free (host);
    return TRUE;
}


static KiroContinueFlag
receive_handler (KiroMessageStatus *status, gpointer user_data)
{
    UcaKiroCamera *cam = (UcaKiroCamera *)user_data;
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (cam);
    
    KiroMessage *msg = status->message;

    if (msg->msg == KIROCS_READY) {
        g_debug ("Interface Setup Done.");
        priv->kiro_connected = TRUE;
        return KIRO_CALLBACK_CONTINUE;
    }

    g_message ("Message Type '%u' is unhandled.", msg->msg);
    return KIRO_CALLBACK_CONTINUE;
}


static void
uca_kiro_camera_clone_interface(UcaKiroCamera *kiro_camera)
{
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (kiro_camera);
    UcaKiroCameraClass *klass = UCA_KIRO_CAMERA_GET_CLASS (kiro_camera);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    if (priv->messenger) {
        kiro_messenger_free (priv->messenger);
        priv->messenger = kiro_messenger_new ();
    }

    priv->kiro_connected = FALSE;

    kiro_messenger_add_receive_callback (priv->messenger, receive_handler, kiro_camera);
    kiro_messenger_connect (priv->messenger, priv->kiro_address, priv->kiro_port, &priv->peer_rank, &initable_iface_error);
    if (initable_iface_error) {
        priv->construction_error = TRUE;
        kiro_messenger_remove_receive_callback (priv->messenger);
        return;
    }

    //Wait until the remote side has given us the "READY" signal
    while (!priv->kiro_connected) {};  

    if (priv->construction_error) {
        //something went wrong. Tear down the connection.
        kiro_messenger_stop (priv->messenger);

        //TODO
        //Maybe set an error?
    }
}


static void
uca_kiro_camera_set_property(GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    g_return_if_fail(UCA_IS_KIRO_CAMERA (object));
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_KIRO_ADDRESS:
            if (priv->kiro_address_string)
                g_free (priv->kiro_address_string);
            priv->kiro_address_string  = g_value_dup_string (value);
            break;
        default:
            g_debug ("Updating %s.", pspec->name); 

            if (!priv->kiro_connected) {
                g_warning ("Trying to modify a property before a connection to the remote camera was established.");
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
                return;
            }

            GError *error = NULL;

            GVariant *tmp = variant_from_scalar (value);
            gsize data_size = g_variant_get_size (tmp);

            PropUpdate *test = g_malloc0 (sizeof (PropUpdate) + data_size);
            test->id = property_id_from_name (pspec->name);
            test->type[0] = gtype_to_gvariant_class (pspec->value_type);
            test->size = data_size;
            g_variant_store (tmp, test->val);
            g_variant_unref (tmp);

            KiroMessage message;
            message.peer_rank = priv->peer_rank;
            message.msg = KIROCS_UPDATE;
            message.payload = test;
            message.size = sizeof (PropUpdate) + data_size;

            kiro_messenger_send_blocking (priv->messenger, &message, &error);
            if (error) {
                g_free (test);
                g_error ("Oh shit! (%s)", error->message);
            }

            g_free (test);
    }
}


static void
uca_kiro_camera_get_property(GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (object);

    switch (property_id) {
        case PROP_NAME:
            g_value_set_string (value, "KIRO camera");
            break;
        case PROP_KIRO_ADDRESS:
            g_value_set_string (value, priv->kiro_address_string);
            break;
        case PROP_KIRO_REMOTE_NAME:
            g_value_set_string (value, priv->remote_name);
            break;
        default:
            //try_handle_read_tango_property (object, property_id, value, pspec);
            break;
    }
}

static void
uca_kiro_camera_finalize(GObject *object)
{
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE(object);

    if (priv->thread_running) {
        priv->thread_running = FALSE;
        g_thread_join (priv->grab_thread);
    }

    if (priv->messenger) {
        kiro_messenger_free (priv->messenger);
        priv->messenger = NULL;
    }
    priv->kiro_connected = FALSE;

    if (priv->dummy_data) {
        g_free (priv->dummy_data);
        priv->dummy_data = NULL;
    }

    g_free (priv->kiro_address_string);
    g_free (priv->kiro_address);
    g_free (priv->kiro_port);

    G_OBJECT_CLASS (uca_kiro_camera_parent_class)->finalize(object);
}

static gboolean
ufo_kiro_camera_initable_init (GInitable *initable,
                               GCancellable *cancellable,
                               GError **error)
{
    g_return_val_if_fail (UCA_IS_KIRO_CAMERA (initable), FALSE);
    
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (UCA_KIRO_CAMERA (initable));
    if(priv->construction_error) {
        g_propagate_error (error, initable_iface_error);
        return FALSE;
    }
    
    return TRUE;
}

static void
uca_kiro_initable_iface_init (GInitableIface *iface)
{
    iface->init = ufo_kiro_camera_initable_init;
}

static void
uca_kiro_camera_constructed (GObject *object)
{
    //Initialization for the KIRO Server and TANGO Interface cloning is moved
    //here and done early!
    //We want to add dynamic properties and it is too late to do so in the
    //real initable part. Therefore, we do it here and 'remember' any errors
    //that occur and check them later in the initable part.
    
    UcaKiroCamera *self = UCA_KIRO_CAMERA (object);
    UcaKiroCameraPrivate *priv = UCA_KIRO_CAMERA_GET_PRIVATE (self);
    priv->construction_error = FALSE;
    
    GValue address = G_VALUE_INIT;
    g_value_init(&address, G_TYPE_STRING);
    uca_kiro_camera_get_property (object, PROP_KIRO_ADDRESS, &address, NULL);

    const gchar *addrstring = g_value_get_string (&address);
    gint address_not_none = g_strcmp0(addrstring, "NONE");

    if (0 == address_not_none) {
        g_warning ("kiro-address was not set! Can not connect to server...\n");
        priv->construction_error = TRUE;
        g_set_error (&initable_iface_error, UCA_KIRO_CAMERA_ERROR, UCA_KIRO_CAMERA_ERROR_MISSING_ADDRESS,
             "'kiro-address' property was not set during construction.");
    }
    else {
        if (kiro_address_decode (addrstring, &priv->kiro_address, &priv->kiro_port, &initable_iface_error)) {
            uca_kiro_camera_clone_interface (self);
        }
        else {
            priv->construction_error = TRUE;
        }
    }

    G_OBJECT_CLASS (uca_kiro_camera_parent_class)->constructed(object);
}



static void
uca_kiro_camera_class_init(UcaKiroCameraClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS( klass);
    gobject_class->set_property = uca_kiro_camera_set_property;
    gobject_class->get_property = uca_kiro_camera_get_property;
    gobject_class->finalize = uca_kiro_camera_finalize;
    gobject_class->constructed = uca_kiro_camera_constructed;

    UcaCameraClass *camera_class = UCA_CAMERA_CLASS (klass);
    camera_class->start_recording = uca_kiro_camera_start_recording;
    camera_class->stop_recording = uca_kiro_camera_stop_recording;
    camera_class->grab = uca_kiro_camera_grab;
    camera_class->trigger = uca_kiro_camera_trigger;

    for (guint i = 0; i < N_BASE_PROPERTIES; i++)
        g_object_class_override_property (gobject_class, i, uca_camera_props[i]);
                
    kiro_properties[PROP_KIRO_ADDRESS] =
        g_param_spec_string("kiro-address",
                "KIRO Server Address",
                "Address of the KIRO Server to grab images from",
                "NONE",
                G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

    kiro_properties[PROP_KIRO_REMOTE_NAME] =
        g_param_spec_string("remote-name",
                "Name of the remot camera",
                "Name of the camera plugin that is loaded on the KIRO remote site",
                "NONE",
                G_PARAM_READABLE);

    for (guint id = N_BASE_PROPERTIES; id < N_PROPERTIES; id++)
        g_object_class_install_property (gobject_class, id, kiro_properties[id]);

    g_type_class_add_private (klass, sizeof(UcaKiroCameraPrivate));
}

static void
uca_kiro_camera_init(UcaKiroCamera *self)
{
    self->priv = UCA_KIRO_CAMERA_GET_PRIVATE(self);
    self->priv->grab_thread = NULL;
    self->priv->current_frame = 0;
    self->priv->kiro_address_string = g_strdup ("NONE");
    self->priv->kiro_address = g_strdup ("NONE");
    self->priv->kiro_port = g_strdup ("NONE");
    self->priv->remote_name = g_strdup ("NONE");
    self->priv->construction_error = FALSE;
    self->priv->kiro_dynamic_attributes = NULL;

    self->priv->messenger = kiro_messenger_new ();
    self->priv->peer_rank = 0;
}


G_MODULE_EXPORT GType
uca_camera_get_type (void)
{
    return UCA_TYPE_KIRO_CAMERA;
}

