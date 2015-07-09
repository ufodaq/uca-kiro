#include "uca/uca-camera.h"
#include "kiro/kiro-messenger.h"

typedef enum {
    KIROCS_UPDATE,
    KIROCS_INSTALL,
    KIROCS_READY,
    KIROCS_RPC,
    KIROCS_EXIT
}KiroCsCommands;

typedef struct {
    guint32 id;
    guint32 size;
    gboolean scalar;
    gchar type[2];
    gchar val[1];
} PropUpdate;

typedef struct {
    guint32 str_len;
    gchar str[1];
}StrProp;

typedef struct {
    GType value_type;
    guint32 name_len;
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
        StrProp str_spec;
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
