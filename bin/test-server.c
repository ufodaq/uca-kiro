#include "kiro-camera-server.h"


static KiroContinueFlag
receive_callback (KiroMessageStatus *status, gpointer user_data)
{
    g_message ("Received a message of type: %u", status->message->msg);
    *(gboolean *)user_data = TRUE;
    return KIRO_CALLBACK_CONTINUE;
}


int main (int argc, char **argv)
{
    GError *error = NULL;

    KiroMessenger *messenger = kiro_messenger_new ();

    gboolean flag = FALSE;
    kiro_messenger_add_receive_callback (messenger, receive_callback, &flag);

    gulong rank = 0;
    kiro_messenger_connect (messenger, "127.0.0.1", "60010", &rank, &error);
    if (error) {
        kiro_messenger_free (messenger);
        g_error ("Oh shit! (%s)", error->message);
    }

    while (!flag) {};
    flag = FALSE;


    GVariant *tmp = g_variant_new ("i", UCA_CAMERA_TRIGGER_SOURCE_SOFTWARE);
    gsize data_size = g_variant_get_size (tmp);

    PropUpdate *test = g_malloc0 (sizeof (PropUpdate) + data_size);
    test->id = 11;
    test->type[0] = 'i';
    test->size = data_size;
    g_variant_store (tmp, test->val);
    //g_object_unref (tmp);

    KiroMessage message;
    message.peer_rank = rank;
    message.msg = KIROCS_UPDATE;
    message.payload = test;
    message.size = sizeof (PropUpdate) + data_size;

    kiro_messenger_send_blocking (messenger, &message, &error);
    if (error) {
        kiro_messenger_free (messenger);
        g_error ("Oh shit! (%s)", error->message);
    }

    message.msg = KIROCS_EXIT;
    message.size = 0;
    message.payload = NULL;

    kiro_messenger_send_blocking (messenger, &message, &error);
    if (error) {
        kiro_messenger_free (messenger);
        g_error ("Oh shit! (%s)", error->message);
    }

    kiro_messenger_free (messenger);

    return 0;



}
