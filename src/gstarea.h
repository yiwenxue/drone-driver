#include <gtk/gtk.h>
#include <gst/gst.h>

struct _gstarea{
    GtkWidget *area;
    GstElement *pipeline;
    GstElement *videosrc;
    GstElement *decoder;
    GstElement *videosink;
    GstStateChangeReturn ret;
    GstElement *converter;
    GstBus *bus;
};
typedef struct _gstarea gst_area_t;

static void destroy_cb (GtkWidget * widget, GdkEvent * event, gst_area_t *gstwidget);
static void end_stream_cb (GstBus * bus, GstMessage * message, gst_area_t *gstwidget);
int gst_area_unref(gst_area_t *widget);
gst_area_t *gst_area_create();
int gst_area_init(gst_area_t *widget);

static void
on_pad_added (GstElement *element,
              GstPad     *pad,
              gpointer    data)
{
  GstPad *sinkpad;
  GstElement *converter = data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  sinkpad = gst_element_get_static_pad (converter, "sink");

  gst_pad_link (pad, sinkpad);

  gst_object_unref (sinkpad);
}

gst_area_t *gst_area_create(){
    gst_area_t * widget = malloc(sizeof(gst_area_t));
    widget->videosrc = gst_element_factory_make ("filesrc", "ximagesrc");

    /* g_object_set(widget->videosrc, "location", "/home/yiwen/Videos/2020-10-21 09-36-42.mkv", NULL); */
    g_object_set(widget->videosrc, "location", "/home/yiwen/Videos/The Ultimate Visual Experience - Drone Racing and FPV Freestyle (My Year 2017)-BfO3oUum-hY.mp4", NULL);
    /* g_object_set(widget->videosrc, "use-damage", 0, NULL); */

    widget->decoder = gst_element_factory_make("decodebin", "decoder");

    widget->converter = gst_element_factory_make("videoconvert", "converter");

    widget->videosink = gst_element_factory_make ("gtksink", "gtksink");

    g_object_get (widget->videosink, "widget", &widget->area, NULL);

    widget->pipeline = gst_pipeline_new ("pipeline");

    if (!widget->pipeline||!widget->area||!widget->decoder||!widget->converter||!widget->videosink||!widget->videosrc)
    {
        g_printerr("One or more element cant be created\n");
    }

    return widget;
}

int gst_area_init(gst_area_t *widget){

    gtk_widget_realize (widget->area);
    
    gst_bin_add_many(GST_BIN (widget->pipeline), widget->videosrc, widget->decoder, widget->converter, widget->videosink, NULL);
    if (!widget->videosrc|| !widget->decoder|| !widget->converter|| !widget->videosink){
        printf("Error!!!\n");
    }

    gst_element_link_pads(widget->videosrc, "src", widget->decoder, "sink");
    gst_element_link_many(widget->converter, widget->videosink, NULL);
    g_signal_connect (widget->decoder, "pad-added", G_CALLBACK (on_pad_added), widget->converter);



    widget->bus = gst_pipeline_get_bus (GST_PIPELINE (widget->pipeline));
    g_signal_connect (widget->bus, "message::error", G_CALLBACK (end_stream_cb),
            widget);
    g_signal_connect (widget->bus, "message::warning", G_CALLBACK (end_stream_cb),
            widget);
    g_signal_connect (widget->bus, "message::eos", G_CALLBACK (end_stream_cb), widget);
    widget->ret = gst_element_set_state (widget->pipeline, GST_STATE_PLAYING);
    if (widget->ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");
        return -1;
    }
    return 0;
}

int gst_area_unref(gst_area_t *widget){

    printf("Hello");
    /* gst_object_unref (widget->bus); */
    /* gst_object_unref (widget->videosink); */
    /* gst_object_unref (widget->videosrc); */
    /* gst_object_unref (widget->converter); */
    /* gst_object_unref (widget->decoder); */
    g_object_unref (widget->area);
    g_object_unref (widget->pipeline);
    printf ("Ends\n");
    free(widget);
    return 0;
}

static void end_stream_cb (GstBus * bus, GstMessage * message, gst_area_t *gstwidget)
{
    g_print ("End of stream\n");

    gst_element_set_state (gstwidget->pipeline, GST_STATE_NULL);
    gst_area_unref(gstwidget);

    gtk_main_quit ();
}
