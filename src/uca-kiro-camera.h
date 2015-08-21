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

#ifndef __UCA_KIRO_CAMERA_H
#define __UCA_KIRO_CAMERA_H

#include <glib-object.h>
#include "uca/uca-camera.h"
#include "kiro/kiro-messenger.h"

G_BEGIN_DECLS

#define UCA_TYPE_KIRO_CAMERA             (uca_kiro_camera_get_type())
#define UCA_KIRO_CAMERA(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), UCA_TYPE_KIRO_CAMERA, UcaKiroCamera))
#define UCA_IS_KIRO_CAMERA(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), UCA_TYPE_KIRO_CAMERA))
#define UCA_KIRO_CAMERA_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), UCA_TYPE_KIRO_CAMERA, UcaKiroCameraClass))
#define UCA_IS_KIRO_CAMERA_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), UCA_TYPE_KIRO_CAMERA))
#define UCA_KIRO_CAMERA_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), UCA_TYPE_KIRO_CAMERA, UcaKiroCameraClass))

#define UCA_KIRO_CAMERA_ERROR    uca_kiro_camera_error_quark()

GQuark uca_kiro_camera_error_quark(void);

typedef enum {
    UCA_KIRO_CAMERA_ERROR_MISSING_ADDRESS = UCA_CAMERA_ERROR_END_OF_STREAM,
    UCA_KIRO_CAMERA_ERROR_ADDRESS_WRONG_FORMAT,
    UCA_KIRO_CAMERA_ERROR_KIRO_CONNECTION_FAILED
} UcaKiroCameraError;


typedef struct _UcaKiroCamera           UcaKiroCamera;
typedef struct _UcaKiroCameraClass      UcaKiroCameraClass;
typedef struct _UcaKiroCameraPrivate    UcaKiroCameraPrivate;

/**
 * UcaKiroCamera:
 *
 * Creates #UcaKiroCamera instances by loading corresponding shared objects. The
 * contents of the #UcaKiroCamera structure are private and should only be
 * accessed via the provided API.
 */
struct _UcaKiroCamera {
    /*< private >*/
    UcaCamera parent;

    UcaKiroCameraPrivate *priv;
};

/**
 * UcaKiroCameraClass:
 *
 * #UcaKiroCamera class
 */
struct _UcaKiroCameraClass {
    /*< private >*/
    UcaCameraClass parent;
};

G_END_DECLS


//HELPER FUNCTIONS AND CONSTRUCTS FOR SERVER AND CAMERA PLUGIN
typedef enum {
    KIROCS_UPDATE,
    KIROCS_FETCH,
    KIROCS_INSTALL,
    KIROCS_READY,
    KIROCS_RPC,
    KIROCS_EXIT
} KiroCsCommands;

typedef enum {
    KIROCS_RPC_START_RECORDING,
    KIROCS_RPC_STOP_RECORDING,
    KIROCS_RPC_START_READOUT,
    KIROCS_RPC_STOP_READOUT,
    KIROCS_RPC_TRIGGER,
    KIROCS_RPC_GRAB,
    KIROCS_RPC_READOUT
//  KIROCS_RPC_WRITE is currently not supported
} KiroCsRPC;

typedef struct {
    guint32 id;
    guint32 size;
    gboolean scalar;
} PropUpdate;

typedef struct {
    PropUpdate base;
    guint64 prop_raw;
} PropUpdateScalar;

typedef struct {
    PropUpdate base;
    gchar str[1];
} PropUpdateString;

typedef struct {
    guint32 id;
    GType value_type;
    union PSpecs {
        GParamSpecBoolean bool_spec;
        GParamSpecChar char_spec;
        GParamSpecInt int_spec;
        GParamSpecUInt uint_spec;
        GParamSpecLong long_spec;
        GParamSpecULong ulong_spec;
        GParamSpecInt64 int64_spec;
        GParamSpecUInt64 uint64_spec;
        GParamSpecFloat float_spec;
        GParamSpecDouble double_spec;
    } spec;
    gchar name[1];
} PropertyRequisition;


//Forward declaration of the trigger enums for type handling
GType uca_camera_trigger_source_get_type (void) G_GNUC_CONST;
#define UCA_TYPE_CAMERA_TRIGGER_SOURCE (uca_camera_trigger_source_get_type ())
GType uca_camera_trigger_type_get_type (void) G_GNUC_CONST;
#define UCA_TYPE_CAMERA_TRIGGER_TYPE (uca_camera_trigger_type_get_type ())


gchar
gtype_to_gvariant_class (GType type)
{
    gchar ret = '*';

    switch (type) {
        case G_TYPE_BOOLEAN:
            ret = G_VARIANT_CLASS_BOOLEAN;
            break;
        case G_TYPE_CHAR:
            ret = G_VARIANT_CLASS_BYTE;
            break;
        case G_TYPE_INT:
            ret = G_VARIANT_CLASS_INT32;
            break;
        case G_TYPE_ENUM:
            ret = G_VARIANT_CLASS_INT32;
            break;
        case G_TYPE_UINT:
            ret = G_VARIANT_CLASS_UINT32;
            break;
        case G_TYPE_LONG:
            ret = G_VARIANT_CLASS_INT64;
            break;
        case G_TYPE_ULONG:
            ret = G_VARIANT_CLASS_UINT64;
            break;
        case G_TYPE_INT64:
            ret = G_VARIANT_CLASS_INT64;
            break;
        case G_TYPE_UINT64:
            ret = G_VARIANT_CLASS_UINT64;
            break;
        case G_TYPE_FLOAT:
            ret = G_VARIANT_CLASS_DOUBLE;
            break;
        case G_TYPE_DOUBLE:
            ret = G_VARIANT_CLASS_DOUBLE;
            break;
        default:
            //ERROR
            break;
    }

    return ret;
}


#define GOBJECT_SET(OBJ, PROP, TYPE, DATA) { \
    g_object_set (OBJ, \
                  PROP, *(TYPE *)DATA, \
                  NULL); \
}

void
update_property_scalar (GObject *cam, const gchar *prop, GType type, gulong handler, gpointer data)
{
    g_debug ("Updating %s, with handler %lu", prop, handler);

    g_signal_handler_block (cam, handler);

    switch (type) {
        case G_TYPE_BOOLEAN:
            GOBJECT_SET (cam, prop, gboolean, data);
            break;
        case G_TYPE_CHAR:
            GOBJECT_SET (cam, prop, gchar, data);
            break;
        case G_TYPE_INT:
            GOBJECT_SET (cam, prop, gint, data);
            break;
        case G_TYPE_ENUM:
            GOBJECT_SET (cam, prop, gint, data);
            break;
        case G_TYPE_UINT:
            GOBJECT_SET (cam, prop, guint, data);
            break;
        case G_TYPE_LONG:
            GOBJECT_SET (cam, prop, glong, data);
            break;
        case G_TYPE_ULONG:
            GOBJECT_SET (cam, prop, gulong, data);
            break;
        case G_TYPE_INT64:
            GOBJECT_SET (cam, prop, gint64, data);
            break;
        case G_TYPE_UINT64:
            GOBJECT_SET (cam, prop, guint64, data);
            break;
        case G_TYPE_FLOAT:
            GOBJECT_SET (cam, prop, gfloat, data);
            break;
        case G_TYPE_DOUBLE:
            GOBJECT_SET (cam, prop, gdouble, data);
            break;
        default:
            //TRIGGER_TYPE and TRIGGER_SOURCE are not statically typed and can
            //not be used in a switch statement...
            if (type == UCA_TYPE_CAMERA_TRIGGER_SOURCE) {
                GOBJECT_SET (cam, prop, gint, data);
                break;
            }

            if (type ==  UCA_TYPE_CAMERA_TRIGGER_TYPE) {
                GOBJECT_SET (cam, prop, gint, data);
                break;
            }

            g_critical ("Type %s not handled! (SET)", g_type_name (type));
            break;
    }

    g_signal_handler_unblock (cam, handler);
}


#define GOBJECT_GET(OBJ, PROP, TYPE, GTYPE) { \
    TYPE tmp; \
    gchar *gvclass = g_malloc0 (2); \
    gvclass[0] = gtype_to_gvariant_class (GTYPE); \
    g_object_get (OBJ, \
                  PROP, &tmp, \
                  NULL); \
    ret = g_variant_new (gvclass, tmp); \
    g_free (gvclass); \
}

GVariant*
read_property_scalar (GObject *cam, const gchar *prop, GType type)
{
    GVariant *ret = NULL;

    switch (type) {
        case G_TYPE_BOOLEAN:
            GOBJECT_GET (cam, prop, gboolean, type);
            break;
        case G_TYPE_CHAR:
            GOBJECT_GET (cam, prop, gchar, type);
            break;
        case G_TYPE_INT:
            GOBJECT_GET (cam, prop, gint, type);
            break;
        case G_TYPE_ENUM:
            GOBJECT_GET (cam, prop, gint, type);
            break;
        case G_TYPE_UINT:
            GOBJECT_GET (cam, prop, guint, type);
            break;
        case G_TYPE_LONG:
            GOBJECT_GET (cam, prop, glong, type);
            break;
        case G_TYPE_ULONG:
            GOBJECT_GET (cam, prop, gulong, type);
            break;
        case G_TYPE_INT64:
            GOBJECT_GET (cam, prop, gint64, type);
            break;
        case G_TYPE_UINT64:
            GOBJECT_GET (cam, prop, guint64, type);
            break;
        case G_TYPE_FLOAT:
            GOBJECT_GET (cam, prop, gfloat, type);
            break;
        case G_TYPE_DOUBLE:
            GOBJECT_GET (cam, prop, gdouble, type);
            break;
        default:
            //TRIGGER_TYPE and TRIGGER_SOURCE are not statically typed and can
            //not be used in a switch statement...
            if (type == UCA_TYPE_CAMERA_TRIGGER_SOURCE) {
                GOBJECT_GET (cam, prop, gint, type);
                break;
            }

            if (type ==  UCA_TYPE_CAMERA_TRIGGER_TYPE) {
                GOBJECT_GET (cam, prop, gint, type);
                break;
            }

            g_critical ("Type %s not handled! (GET)", g_type_name (type));
            break;
    }

    return ret;
}



#define GVALUE_TO_GVARIANT(VALUE, FUNC, TYPE, GTYPE) { \
    TYPE tmp; \
    gchar *gvclass = g_malloc0 (2); \
    gvclass[0] = gtype_to_gvariant_class (GTYPE); \
    tmp = FUNC (VALUE); \
    ret = g_variant_new (gvclass, tmp); \
    g_free (gvclass); \
}

GVariant*
variant_from_scalar (GValue *value)
{
    GVariant *ret = NULL;

    GType type = G_VALUE_TYPE (value);

    switch (type) {
        case G_TYPE_BOOLEAN:
            GVALUE_TO_GVARIANT (value, g_value_get_boolean, gboolean, type);
            break;
        case G_TYPE_CHAR:
            GVALUE_TO_GVARIANT (value, g_value_get_char, gchar, type);
            break;
        case G_TYPE_INT:
            GVALUE_TO_GVARIANT (value, g_value_get_int, gint, type);
            break;
        case G_TYPE_ENUM:
            GVALUE_TO_GVARIANT (value, g_value_get_int, gint, type);
            break;
        case G_TYPE_UINT:
            GVALUE_TO_GVARIANT (value, g_value_get_uint, guint, type);
            break;
        case G_TYPE_LONG:
            GVALUE_TO_GVARIANT (value, g_value_get_long, glong, type);
            break;
        case G_TYPE_ULONG:
            GVALUE_TO_GVARIANT (value, g_value_get_ulong, gulong, type);
            break;
        case G_TYPE_INT64:
            GVALUE_TO_GVARIANT (value, g_value_get_int64, gint64, type);
            break;
        case G_TYPE_UINT64:
            GVALUE_TO_GVARIANT (value, g_value_get_uint64, guint64, type);
            break;
        case G_TYPE_FLOAT:
            GVALUE_TO_GVARIANT (value, g_value_get_float, gfloat, type);
            break;
        case G_TYPE_DOUBLE:
            GVALUE_TO_GVARIANT (value, g_value_get_double, gdouble, type);
            break;
        default:
            //TRIGGER_TYPE and TRIGGER_SOURCE are not statically typed and can
            //not be used in a switch statement...
            if (type == UCA_TYPE_CAMERA_TRIGGER_SOURCE) {
                GVALUE_TO_GVARIANT (value, g_value_get_int, gint, type);
                break;
            }

            if (type ==  UCA_TYPE_CAMERA_TRIGGER_TYPE) {
                GVALUE_TO_GVARIANT (value, g_value_get_int, gint, type);
                break;
            }

            g_critical ("Type %s not handled! (GET)", g_type_name (type));
            break;
    }

    return ret;
}


guint
property_id_from_name(const gchar* name, guint n_props, GParamSpec **props)
{
    guint idx = 0;
    gboolean found = FALSE;
    for (;idx < n_props; ++idx) {
        if (0 == g_strcmp0(name, props[idx]->name)) {
            found = TRUE;
            break;
        }
    }
    return found ? (idx + 1) : 0;
}


void
g_value_write_to_raw_data (const GValue *value, gpointer raw)
{
    GType type = G_VALUE_TYPE (value);

    switch (type) {
        case G_TYPE_BOOLEAN:
            *(gboolean *)raw = g_value_get_boolean (value);
            break;
        case G_TYPE_CHAR:
            *(gchar *)raw = g_value_get_char (value);
            break;
        case G_TYPE_INT:
            *(gint *)raw = g_value_get_int (value);
            break;
        case G_TYPE_ENUM:
            *(gint *)raw = g_value_get_enum (value);
            break;
        case G_TYPE_UINT:
            *(guint *)raw = g_value_get_uint (value);
            break;
        case G_TYPE_LONG:
            *(glong *)raw = g_value_get_long (value);
            break;
        case G_TYPE_ULONG:
            *(gulong *)raw = g_value_get_ulong (value);
            break;
        case G_TYPE_INT64:
            *(gint64 *)raw = g_value_get_int64 (value);
            break;
        case G_TYPE_UINT64:
            *(guint64 *)raw = g_value_get_uint64 (value);
            break;
        case G_TYPE_FLOAT:
            *(gfloat *)raw = g_value_get_float (value);
            break;
        case G_TYPE_DOUBLE:
            *(gdouble *)raw = g_value_get_double (value);
            break;
        default:
            //TRIGGER_TYPE and TRIGGER_SOURCE are not statically typed and can
            //not be used in a switch statement...
            if (type == UCA_TYPE_CAMERA_TRIGGER_SOURCE) {
                *(gint *)raw = g_value_get_int (value);
                break;
            }

            if (type ==  UCA_TYPE_CAMERA_TRIGGER_TYPE) {
                *(gint *)raw = g_value_get_int (value);
                break;
            }

            g_critical ("Type %s not handled! (GET)", g_type_name (type));
            break;
    }
}


void
g_value_set_from_raw_data (GValue *value, gpointer raw)
{
    GType type = G_VALUE_TYPE (value);

    switch (type) {
        case G_TYPE_BOOLEAN:
            g_value_set_boolean (value, *(gboolean *)raw);
            break;
        case G_TYPE_CHAR:
            g_value_set_char (value, *(gchar *)raw);
            break;
        case G_TYPE_INT:
            g_value_set_int (value, *(gint *)raw);
            break;
        case G_TYPE_ENUM:
            g_value_set_enum (value, *(gint *)raw);
            break;
        case G_TYPE_UINT:
            g_value_set_uint (value, *(guint *)raw);
            break;
        case G_TYPE_LONG:
            g_value_set_long (value, *(glong *)raw);
            break;
        case G_TYPE_ULONG:
            g_value_set_ulong (value, *(gulong *)raw);
            break;
        case G_TYPE_INT64:
            g_value_set_int64 (value, *(gint64 *)raw);
            break;
        case G_TYPE_UINT64:
            g_value_set_uint64 (value, *(guint64 *)raw);
            break;
        case G_TYPE_FLOAT:
            g_value_set_float (value, *(gfloat *)raw);
            break;
        case G_TYPE_DOUBLE:
            g_value_set_double (value, *(gdouble *)raw);
            break;
        default:
            //TRIGGER_TYPE and TRIGGER_SOURCE are not statically typed and can
            //not be used in a switch statement...
            if (type == UCA_TYPE_CAMERA_TRIGGER_SOURCE) {
                g_value_set_enum (value, *(gint *)raw);
                break;
            }

            if (type ==  UCA_TYPE_CAMERA_TRIGGER_TYPE) {
                g_value_set_enum (value, *(gint *)raw);
                break;
            }

            g_critical ("Type %s not handled! (SET)", g_type_name (type));
            break;
    }
}



#endif
