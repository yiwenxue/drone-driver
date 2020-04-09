#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>

#include <clutter-1.0/clutter/clutter.h>
#include <unistd.h>

#include "dronesimulation-window.h"

#define MARKER_SIZE 10

double lat = 32.068548;
double lon = 34.836533;
gboolean icenter = FALSE;
gboolean imapfloat = TRUE;

gboolean imapfloat_signal_lock = FALSE;

GtkWidget *mapviewport;
GtkWidget *videoviewport;
GtkWidget *mainbox;
GtkWidget *floatbox;
GtkBuilder *builder;
GtkWidget *clutter;

typedef struct{
  ChamplainView *view;
  ChamplainMarker *marker;
} GpsCallbackData;

static gboolean
gps_callback (GpsCallbackData *data)
{
    lat += 0.00001;
    lon += 0.00001;
    if (icenter)
        champlain_view_center_on (data->view, lat, lon);
    champlain_location_set_location (CHAMPLAIN_LOCATION (data->marker), lat, lon);
    return TRUE;
}

static gboolean
ifcenter_toggled_cb ()
{
    if(icenter == TRUE) 
        icenter = FALSE;
    else 
        icenter = TRUE;
    return TRUE;
}

static gboolean
ifmapfloat_clicked_cb (  )
{
    if( !imapfloat_signal_lock ){
        imapfloat_signal_lock = TRUE;
        GtkWidget *List;
        if( imapfloat == TRUE ){
            imapfloat = FALSE;
            g_object_ref(G_OBJECT(mapviewport));
            gtk_container_remove(GTK_CONTAINER(floatbox), mapviewport);
            /* gtk_container_remove(GTK_CONTAINER(floatbox), videoviewport); */
            gtk_container_add(GTK_CONTAINER(mainbox), mapviewport);
            g_object_unref(G_OBJECT(mapviewport));

            g_object_ref(G_OBJECT(videoviewport));
            gtk_container_remove(GTK_CONTAINER(mainbox), videoviewport);
            /* gtk_container_remove(GTK_CONTAINER(mainbox), mapviewport); */
            gtk_container_add(GTK_CONTAINER(floatbox), videoviewport);
            g_object_unref(G_OBJECT(videoviewport));

        }else{
            imapfloat = TRUE;

            g_object_ref(G_OBJECT(videoviewport));
            /* gtk_container_remove(GTK_CONTAINER(floatbox), mapviewport); */
            gtk_container_remove(GTK_CONTAINER(floatbox), videoviewport);
            gtk_container_add(GTK_CONTAINER(mainbox), videoviewport);
            g_object_unref(G_OBJECT(videoviewport));

            g_object_ref(G_OBJECT(mapviewport));
            gtk_container_remove(GTK_CONTAINER(mainbox), mapviewport);
            /* gtk_container_remove(GTK_CONTAINER(mainbox), videoviewport); */
            gtk_container_add(GTK_CONTAINER(floatbox), mapviewport);
            g_object_unref(G_OBJECT(mapviewport));
        }
        imapfloat_signal_lock = FALSE;
        printf("Change view\n");
        return TRUE;
    }
    else {
        printf("view not changeed\n");
        return FALSE;
    }
}


static gboolean
mouse_click_cb (ClutterActor *actor, ClutterButtonEvent *event, ChamplainView *view)
{
  gdouble lat, lon;

  lon = champlain_view_x_to_longitude (view, event->x);
  lat = champlain_view_y_to_latitude (view, event->y);
  g_print ("Mouse click at: %f  %f\n", lat, lon);

  return TRUE;
}

int
main (int argc, char *argv[])
{
    GtkWidget *window;

    GtkWidget *ifcenter;
    GtkWidget *ifmapfloat;

    /* GtkBuilder *builder; */
    GError *error = NULL;

    ChamplainView *view;
    ClutterActor *marker, *scale;
    ChamplainMarkerLayer *layer;
    GpsCallbackData position_data;

    if (gtk_clutter_init(&argc, &argv) != CLUTTER_INIT_SUCCESS)
        return 1;

    {

        /* Create a builder and load widgets from resource file */
        builder = gtk_builder_new();
        if( gtk_builder_add_from_resource(builder, "/com/github/yiwenxue/dronesimulation/ui/dronesimulation-window.ui", &error) == 0 )
        {
            g_printerr ("Error loading file: %s\n", error->message);
            g_clear_error (&error);
            return 1;
        }

        window = gtk_builder_get_object(builder, "mainwindow");

        clutter = gtk_champlain_embed_new();

        mapviewport = gtk_builder_get_object(builder, "map_viewport");
        videoviewport = gtk_builder_get_object(builder, "video_viewport");
        mainbox = gtk_builder_get_object(builder, "main_box");
        floatbox = gtk_builder_get_object(builder, "float_box");

        ifcenter = gtk_builder_get_object(builder, "ifcenter");

        ifmapfloat = gtk_builder_get_object(builder, "ifmapfloat");

        view = gtk_champlain_embed_get_view (GTK_CHAMPLAIN_EMBED (clutter));

        scale = champlain_scale_new ();

        marker = create_marker ();
    }

    {
        /* Create the map view */
        clutter_actor_set_reactive (CLUTTER_ACTOR (view), TRUE);

        /* Finish initialising the map view */
        g_object_set (G_OBJECT (view), "zoom-level", 16, "kinetic-mode", TRUE, NULL);
        /* /1* clutter_actor_set_size (CLUTTER_ACTOR (view), 900, 700); *1/ */

        champlain_view_center_on (CHAMPLAIN_VIEW (view), lat, lon);
        g_object_set_data (G_OBJECT (view), "viewpoint", mapviewport);

        /* /1* Create the marker layer *1/ */
        layer = champlain_marker_layer_new_full (CHAMPLAIN_SELECTION_SINGLE);
        clutter_actor_show (CLUTTER_ACTOR (layer));

        /* /1* Create a marker *1/ */
        champlain_marker_layer_add_marker (layer, CHAMPLAIN_MARKER (marker));

        champlain_view_add_layer (view, CHAMPLAIN_LAYER (layer));
        clutter_actor_set_size(CLUTTER_ACTOR(view),10,10);
    }

    {
        gtk_container_add(GTK_CONTAINER(mainbox), videoviewport);
        gtk_container_add(GTK_CONTAINER(floatbox), mapviewport);

        /* Create callback that updates the map periodically */
        position_data.view = CHAMPLAIN_VIEW (view);
        position_data.marker = CHAMPLAIN_MARKER (marker);

        g_timeout_add (500, (GSourceFunc) gps_callback, &position_data);

        // Signal handler
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

        gtk_container_add(GTK_CONTAINER(mapviewport), clutter);
        

        g_signal_connect (GTK_TOGGLE_BUTTON (ifcenter), "toggled",
                G_CALLBACK (ifcenter_toggled_cb), NULL);

        g_signal_connect (GTK_BUTTON (ifmapfloat), "clicked",
                G_CALLBACK (ifmapfloat_clicked_cb), NULL);

        g_signal_connect (view, "button-release-event", G_CALLBACK (mouse_click_cb), view);
    }



    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
