#include <stdlib.h>
#include <math.h>
#include <champlain/champlain.h>
#include <gtk/gtk.h>

int main(int argc, char *argv[])
{
    GtkBuilder *builder;
    GObject *window;
    GError *error = NULL;
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    if(gtk_builder_add_from_resource(builder, "/com/github/yiwenxue/dronesimulation/ui/dronesimulation-window.ui", &error) == 0)
    {
        g_printerr("Error loading file:%s\n",error->message);
        g_clear_error(&error);
        return -1;
    }

    window = gtk_builder_get_object(builder, "mainwindow");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

    gtk_main();
    return 0;
}
