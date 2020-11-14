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

#include "cairo.h"
#include "dronesimulation-window.h"
#include "gstarea.h"

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
            g_object_ref(G_OBJECT(videoviewport));

            gtk_container_remove(GTK_CONTAINER(floatbox), mapviewport);
            gtk_container_remove(GTK_CONTAINER(mainbox), videoviewport);

            gtk_container_add(GTK_CONTAINER(mainbox), mapviewport);
            gtk_container_add(GTK_CONTAINER(floatbox), videoviewport);

            g_object_unref(G_OBJECT(mapviewport));
            g_object_unref(G_OBJECT(videoviewport));

        }else{
            imapfloat = TRUE;

            g_object_ref(G_OBJECT(videoviewport));
            g_object_ref(G_OBJECT(mapviewport));

            gtk_container_remove(GTK_CONTAINER(floatbox), videoviewport);
            gtk_container_remove(GTK_CONTAINER(mainbox), mapviewport);

            gtk_container_add(GTK_CONTAINER(mainbox), videoviewport);
            gtk_container_add(GTK_CONTAINER(floatbox), mapviewport);

            g_object_unref(G_OBJECT(videoviewport));
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

double start = 5;
double end = 20;
double roll = 0;
double THETA = 0;

typedef struct{
    cairo_t *cr;

    double posx;
    double posy;

    double llength;
    double start;
    double end;
    double tics;
    double tics_length;
    int tics_level;
    char ori;
    int tics_ori;
    double linewidth;
    double fontsize;

    char mark;
    double markpos;
} coord;

enum {
    TICS_ORI_RIGHT = 1,
    TICS_ORI_LEFT = -1, 
    TICS_ORI_UP = -1,
    TICS_ORI_DOWN = 1
};


int draw_coord(coord *data)
{
    cairo_select_font_face(data->cr, "Purisa",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(data->cr, data->fontsize);

    char string[255];

    double ini_linew = cairo_get_line_width(data->cr);
    cairo_set_line_width(data->cr, data->linewidth);

    double head_tics = 1.2;

    cairo_set_line_cap(data->cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(data->cr, data->posx, data->posy);
    double step_length = 1.0 / (data->end - data->start) * data->llength;
    if (data->end - data->start <= data->tics ){
        fprintf(stderr, "tics length smaller than whole length\n");
        return -1;
    }

    double tics;
    double tics_end;
    double tics_start;
    double tics_pos;
    double text_align;

    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    cairo_font_extents (data->cr, &fe);

    if(data->ori == 'x'){
        cairo_rel_line_to(data->cr, 0, data->tics_ori * data->tics_length * head_tics);
        cairo_move_to(data->cr, data->posx, data->posy);
        cairo_rel_line_to(data->cr, data->llength, 0);
        cairo_rel_line_to(data->cr, 0, data->tics_ori * data->tics_length * head_tics);

        if (data->mark != 0){
            cairo_move_to(data->cr, data->posx + data->llength / 2.0, data->posy + data->tics_length * 0.9 * data->tics_ori);
            cairo_rel_line_to(data->cr, data->tics_length * 0.5 / 1.7, data->tics_length * 0.5 * data->tics_ori);
            cairo_rel_line_to(data->cr, data->tics_length / -1.7, 0);
            cairo_close_path(data->cr);
        }
        
        cairo_stroke(data->cr);

        cairo_move_to(data->cr, data->posx, data->posy);
        for (int i = 1; i <= data->tics_level; ++i) {
            tics = data->tics /   pow(5.0, i-1);

            if(data->start < 0)
                tics_start = data->posx + ( (int)(data->start/tics) * (tics) - data->start) * step_length;
            else
                tics_start = data->posx + ( (int)(data->start/tics) * (tics) + (tics) - data->start) * step_length;
            tics_end = data->posx + data->llength;

            for(tics_pos = tics_start; tics_pos <= tics_end; tics_pos += tics*step_length)
            {
                cairo_move_to(data->cr, tics_pos, data->posy);
                cairo_set_line_width(data->cr, data->linewidth * pow(0.7,i));
                cairo_rel_line_to(data->cr, 0, data->tics_ori * data->tics_length * pow(0.7,i));
                if ( i == 1 ){
                    if (data->tics_ori == TICS_ORI_DOWN)
                        text_align = fe.descent;
                    else 
                        text_align =  fe.height;
                    cairo_rel_move_to(data->cr, -1 * data->tics_length * 0.7, data->tics_ori * (data->tics_length *  -1 * pow(0.7,i) - text_align));
                    /* cairo_rel_move_to(data->cr, -1 * data->tics_length * 0.7, data->tics_ori * data->tics_length * -1 * pow(0.7,i) - text_align); */
                    sprintf(string, "%.2f", (tics_pos - data->posx)/step_length + data->start);
                    cairo_show_text(data->cr, string);
                }
            }
            cairo_stroke(data->cr);

        }

    }else if(data->ori == 'y'){
        cairo_rel_line_to(data->cr, data->tics_ori * data->tics_length * head_tics, 0);
        cairo_move_to(data->cr, data->posx, data->posy);
        cairo_rel_line_to(data->cr, 0, -data->llength);
        cairo_rel_line_to(data->cr, data->tics_ori * data->tics_length * head_tics, 0);

        if (data->mark != 0){
            cairo_move_to(data->cr, data->posx + data->tics_length * 0.9 * data->tics_ori, data->posy - data->llength / 2.0);
            cairo_rel_line_to(data->cr, data->tics_length * 0.5 * data->tics_ori, data->tics_length * 0.5 / 1.7);
            cairo_rel_line_to(data->cr, 0, data->tics_length / -1.7);
            cairo_close_path(data->cr);
        }
        cairo_stroke(data->cr);

        cairo_move_to(data->cr, data->posx, data->posy);
        for (int i = 1; i <= data->tics_level; ++i) {
            tics = data->tics /   pow(5.0, i-1);

            if(data->start < 0)
                tics_start = data->posy - ( (int)(data->start/tics) * (tics) - data->start) * step_length;
            else
                tics_start = data->posy - ( (int)(data->start/tics) * (tics) + (tics) - data->start) * step_length;
            tics_end = data->posy - data->llength;

            for(tics_pos = tics_start; tics_pos >= tics_end; tics_pos -= tics*step_length)
            {
                cairo_move_to(data->cr, data->posx, tics_pos);
                cairo_set_line_width(data->cr, data->linewidth * pow(0.7,i));
                cairo_rel_line_to(data->cr, data->tics_ori * data->tics_length * pow(0.7,i), 0);
                if ( i == 1 ){
                    sprintf(string, "%.2f", -1 * (tics_pos - data->posy)/step_length + data->start);

                    if(data->tics_ori == TICS_ORI_LEFT){
                        cairo_text_extents(data->cr, "a", &te);
                        text_align = - te.width;
                    }else if(data->tics_ori == TICS_ORI_RIGHT){
                        cairo_text_extents(data->cr, string, &te);
                        text_align = - te.width - te.height;
                    }else{continue;}
                    cairo_rel_move_to(data->cr, data->tics_ori * (data->tics_length * -1 * pow(0.7,i) + text_align)   , fe.height / 2.0);
                    cairo_show_text(data->cr, string);
                }
            }
            cairo_stroke(data->cr);
        }
    }

    cairo_set_line_width(data->cr, ini_linew);
    return 0;
}


static void do_drawing(cairo_t *, GtkWidget *widget);

static gboolean on_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
    return TRUE;
}

static gboolean on_expose_event(GtkWidget *widget, cairo_t *cr, 
    gpointer user_data)
{        
    do_drawing(cr, widget);
    gtk_widget_queue_draw(widget);

  return FALSE;
}

static void 
draw_box(cairo_t *cr, double x, double y, double width, double height, double ticsl){
    cairo_set_source_rgba(cr, 0, 255, 0, 0.6);

    double ticslx =0, ticsly=0;
    ticslx = ticsly = ticsl;
    if (ticsl == 0){
        ticslx = width / 2.;
        ticsly = height / 2.;
    }else 
        ticslx = ticsly = ticsl;

    cairo_move_to(cr, x - width/2., y - height/2.);
    cairo_rel_line_to(cr, ticslx, 0);
    cairo_move_to(cr, x + width/2., y - height/2.);
    cairo_rel_line_to(cr, -ticslx, 0);

    cairo_move_to(cr, x - width/2., y + height/2.);
    cairo_rel_line_to(cr, ticslx, 0);
    cairo_move_to(cr, x + width/2., y + height/2.);
    cairo_rel_line_to(cr, -ticslx, 0);

    cairo_move_to(cr, x - width/2., y - height/2.);
    cairo_rel_line_to(cr, 0, ticsly);
    cairo_move_to(cr, x + width/2., y - height/2.);
    cairo_rel_line_to(cr, 0, ticsly);

    cairo_move_to(cr, x - width/2., y + height/2.);
    cairo_rel_line_to(cr, 0, -ticsly);
    cairo_move_to(cr, x + width/2., y + height/2.);
    cairo_rel_line_to(cr, 0, -ticsly);

    cairo_stroke(cr);
}

static void
drone_draw_curser(cairo_t *cr, double x, double y, double cursel){
    //Draw curse: 
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.8);
    cairo_move_to(cr, x, y - cursel);
    cairo_rel_line_to(cr, 0, cursel *2);
    cairo_move_to(cr, x - cursel, y);
    cairo_rel_line_to(cr, cursel *2, 0);
    cairo_stroke(cr);
}

struct pole_coord{
    double start;
    double end;
    double centerx;
    double centery;
    double radius;
    double startR;
    double endR;
    double tics;
    double tics_ori;
    double linewidth;
    double tics_length;
    double tics_level;
    unsigned char tics_text;
};

enum pole_ori{
    POLE_ORI_OUTTER = 1,
    POLE_ORI_INNER = -1
};

static void 
pole_axis_draw(cairo_t *cr, struct pole_coord *coord){
    cairo_set_line_width(cr, coord->linewidth);
    cairo_arc(cr, coord->centerx, coord->centery, 
            coord->radius, coord->startR, coord->endR);
    /* printf("startR: %f\tendR: %f\tradius: %f\n", coord->startR, coord->endR, coord->radius); */
    cairo_set_font_size(cr, coord->linewidth* 5.0);

    double head_tics = 1.1;

    cairo_stroke(cr);
    double step = coord->endR - coord->startR;

    cairo_move_to(cr, coord->centerx, coord->centery);
    cairo_rotate(cr, coord->startR);
    cairo_rel_move_to(cr, coord->radius, 0);
    cairo_rel_line_to(cr, coord->tics_length * coord->tics_ori * head_tics, 0);
    cairo_rotate(cr, -1*coord->startR);

    cairo_move_to(cr, coord->centerx, coord->centery);
    cairo_rotate(cr, coord->endR);
    cairo_rel_move_to(cr, coord->radius, 0);
    cairo_rel_line_to(cr, coord->tics_length * coord->tics_ori * head_tics, 0);
    cairo_rotate(cr, -1*coord->endR);

    double tics, tics_start, tics_end, tics_deg;
    double step_length = 1.0/ (coord->end - coord->start) * (coord->endR- coord->startR);

    /* printf("start: %f\tend: %f\tstep: %f\n", tics_start, tics_end, step_length); */

    char string[255];
    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    double text_align;
    cairo_font_extents (cr, &fe);


    for (int i = 1; i <= coord->tics_level; ++i) {
        tics = coord->tics / pow(5.0, i-1);

        if (coord->start < 0)
            tics_start = coord->startR+ ((int ) (coord->start/tics) * tics - coord->start ) * step_length;
        else 
            tics_start = coord->startR+ ((int ) (coord->start/tics) * tics + tics - coord->start ) * step_length;
        tics_end = coord->endR + step_length * tics * 1e-10;

        /* printf("start: %f\tend: %f\tstep: %f\n", tics_start, tics_end, step_length); */

        for (tics_deg = tics_start; tics_deg <= tics_end; tics_deg += tics * step_length) {
            cairo_set_line_width(cr, coord->linewidth * pow(0.7,i-1));
            cairo_move_to(cr, coord->centerx, coord->centery);
            cairo_rotate(cr, tics_deg);
            cairo_rel_move_to(cr, coord->radius, 0);
            cairo_rel_line_to(cr, coord->tics_length * coord->tics_ori * pow(0.7,i-1), 0);
            cairo_rotate(cr, M_PI*0.5);
            if (i == 1 && coord->tics_text){
                double value = (tics_deg - coord->startR)/step_length + coord->start;
                sprintf(string, "%3.2f", value == -0 ? 0 : value);
                cairo_text_extents(cr, "a", &te);
                text_align = - te.height;
                cairo_rel_move_to(cr, 0, text_align);
                cairo_text_extents(cr, string, &te);
                text_align = -0.5 * te.width;
                cairo_rel_move_to(cr, text_align, 0);
                cairo_show_text(cr, string);
            }
            cairo_rotate(cr, M_PI*-0.5);
            cairo_rotate(cr, -1*tics_deg);
        }
    }

    cairo_stroke(cr);
}

static void
drone_draw_roll(cairo_t *cr, double width, double height){
    cairo_set_source_rgba(cr, 0, 255, 0, 0.6);
    double minimum = height > width ? width: height;
    double radius = minimum * 0.3;
    roll += (rand()%200 - 100)/50000.0 * M_PI;
    // Draw roll plate:
    struct pole_coord plcoord;
    plcoord.start = -60;
    plcoord.end = 60;
    plcoord.centerx = width * .5;
    plcoord.centery = height * .5;
    plcoord.startR = roll - M_PI*5/6.;
    plcoord.endR = roll - M_PI*1/6.;
    plcoord.radius = radius;
    plcoord.tics_length = minimum * 0.02;
    plcoord.tics_ori = POLE_ORI_OUTTER;
    plcoord.tics = 15;
    plcoord.tics_text = 0;
    plcoord.tics_level = 2;
    plcoord.linewidth = width / 500.0;
    pole_axis_draw(cr, &plcoord);
    cairo_move_to(cr, plcoord.centerx, plcoord.centery - radius * 0.95);
    double len = minimum* 0.02;
    cairo_set_line_width(cr, minimum * 0.002);
    cairo_rel_line_to(cr, len / 1.7 * 0.5, len *0.5);
    cairo_rel_line_to(cr, -1*len / 1.7, 0);
    cairo_close_path(cr);
    cairo_stroke(cr);

}

static void
drone_draw_axis(cairo_t *cr, double width, double height){
    // Draw Border axis
    cairo_set_source_rgba(cr, 0, 255, 0, 0.6);
    coord mcoord;
    mcoord.cr = cr;
    mcoord.mark = 1;

    double mid = start;
    mcoord.start = mid - 45;
    mcoord.end = mid + 45;
    // Draw bottom axis: To be determined
    mcoord.posx = 0.3*width;
    mcoord.posy = 0.9*height;
    mcoord.llength = 0.4* width;
    mcoord.tics = 15;
    mcoord.tics_length = height*0.02;
    mcoord.ori = 'x';
    mcoord.tics_ori = 1.0;
    mcoord.tics_level = 2;
    mcoord.tics_ori = TICS_ORI_UP;
    mcoord.linewidth = width / 500.0;
    mcoord.fontsize = mcoord.linewidth * 5;
    draw_coord(&mcoord);

    // Draw top bar: radius.
    mcoord.posx = 0.2*width;
    mcoord.posy = 0.1*height;
    mcoord.llength = 0.6* width;
    mcoord.ori = 'x';
    mcoord.tics_level = 3;
    mcoord.tics_ori = TICS_ORI_DOWN;
    draw_coord(&mcoord);

    // Left hand side axis: The height 
    mid = end;
    end += 0.5;
    mcoord.start = mid - 200;
    mcoord.end = mid + 200;
    mcoord.tics_level = 2;
    mcoord.posx = 0.15*width;
    mcoord.posy = 0.6*height;
    mcoord.ori = 'y';
    mcoord.tics = 50;
    mcoord.llength = 0.4* height;
    mcoord.tics_ori = TICS_ORI_RIGHT;
    draw_coord(&mcoord);

    mcoord.posx = 0.85*width;
    mcoord.posy = 0.6*height;
    mcoord.ori = 'y';
    mcoord.tics_ori = TICS_ORI_LEFT;
    draw_coord(&mcoord);

}

static void
drone_draw_clean(cairo_t *cr, double width, double height){
    cairo_set_source_rgba(cr, 0, 0, 0, 0.2);

    cairo_move_to(cr, 0, 0);
    cairo_rel_line_to(cr, width, 0);
    cairo_rel_line_to(cr, 0, height);
    cairo_rel_line_to(cr, -width, height);
    cairo_close_path(cr);
    cairo_fill(cr);
}

static void
drone_draw_battery(cairo_t * cr, double x, double y, double size, double cap)
{
    x += size * .15;
    y += size * .85;
    size = size * 0.7;
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, size * 2, 0);
    cairo_rel_line_to(cr, 0, -1*size);
    cairo_rel_line_to(cr, size * -2, 0);
    cairo_rel_line_to(cr, 0, size);

    cairo_move_to(cr, x+size*2, y-size*0.7);
    cairo_rel_line_to(cr, size*.2, 0);
    cairo_rel_line_to(cr, 0, size*.4);
    cairo_rel_line_to(cr, size*-.2, 0);
    cairo_stroke(cr);

    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, size * 0.02 * cap, 0);
    cairo_rel_line_to(cr, 0, -1*size);
    cairo_rel_line_to(cr, size * -.02 * cap, 0);
    cairo_rel_line_to(cr, 0, size);
    cairo_fill(cr);
}

static void
text_box(cairo_t *cr, double x, double y, char *msg)
{
    cairo_font_extents_t fe;
    cairo_text_extents_t te;
    cairo_font_extents (cr, &fe);
    cairo_text_extents(cr, msg, &te);
    double width  = te.width  + 2* te.height;
    double height = te.height + 2* (te.y_advance - te.y_bearing);
    
    /* draw_box(cr, x, y, width, height, 0); */
    cairo_move_to(cr, x, y);
    cairo_rel_line_to(cr, width, 0);
    cairo_rel_line_to(cr, 0, height);
    cairo_rel_line_to(cr, -1*width, 0);
    cairo_close_path(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, x, y);
    
    cairo_rel_move_to(cr, te.height, te.height-te.y_bearing);
    cairo_show_text(cr, msg);
    cairo_stroke(cr);
}



static void do_drawing(cairo_t *cr, GtkWidget *widget)
{         
    GtkWidget *win = gtk_widget_get_parent(widget);

    start += 0.1;
    if (start >= 180)
        start -= 360;

    gint width, height;
    /* gtk_widget_get_size_request(win, &width, &height); */
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    double minimum = height > width ? width: height;


    char string[255];
    cairo_stroke(cr);

    cairo_text_extents_t te;
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // clean the plate before the new plot
    //
    drone_draw_clean(cr, width, height);

    // Plot the center box
    draw_box(cr, width*0.5, height*0.5, width*0.15, height*0.15, 20);
    drone_draw_curser(cr, width*0.5, height*0.5, 20);

    drone_draw_battery(cr, 5, 5, minimum/48, 80);

    /* text_box(cr, width-100, 5, "Drone console"); */

    // Draw main axises 
    drone_draw_axis(cr, width, height);

    // Plot the roll plate
    drone_draw_roll(cr, width, height);

    // Expose the plot to surface
    cairo_stroke(cr);
}

static gboolean
anim_callback (void *ptr)
{
    THETA += 0.01 * M_PI;
    return TRUE;
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

static void destroy_cb (GtkWidget * widget, GdkEvent * event, gst_area_t *gstwidget)
{
    g_print ("Close\n");

    gst_element_set_state (gstwidget->pipeline, GST_STATE_NULL);
    gst_area_unref(gstwidget);

    gtk_main_quit ();
}

int
main (int argc, char *argv[])
{
    GtkWidget *window;

    GtkWidget *ifcenter;
    GtkWidget *ifmapfloat;
    GtkWidget *draw_curse;
    GtkWidget *video_pane;
    GtkBuilder *builder;
    GError *error = NULL;

    ChamplainView *view;
    ClutterActor *marker, *scale;
    ChamplainMarkerLayer *layer;
    GpsCallbackData position_data;

    srand((unsigned int) clock());
    gst_area_t *gstarea;

    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

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

        /* gtk_window_set_resizable (window, FALSE); */
        clutter = gtk_champlain_embed_new();

        mapviewport = gtk_builder_get_object(builder, "map_viewport");
        videoviewport = gtk_builder_get_object(builder, "video_viewport");
        mainbox = gtk_builder_get_object(builder, "main_box");
        floatbox = gtk_builder_get_object(builder, "float_box");
        draw_curse = GTK_DRAWING_AREA(gtk_builder_get_object(builder, "draw_curse"));
        video_pane = gtk_builder_get_object (builder, "video_pane");

        gstarea = gst_area_create ();
        gtk_box_pack_start (GTK_BOX (video_pane), gstarea->area, FALSE, FALSE, 0);
        gst_area_init (gstarea);

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
        /* g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL); */
        g_signal_connect (window, "delete-event", G_CALLBACK (destroy_cb), gstarea);

        gtk_container_add(GTK_CONTAINER(mapviewport), clutter);
        

        g_signal_connect (GTK_TOGGLE_BUTTON (ifcenter), "toggled",
                G_CALLBACK (ifcenter_toggled_cb), NULL);

        g_signal_connect (GTK_BUTTON (ifmapfloat), "clicked",
                G_CALLBACK (ifmapfloat_clicked_cb), NULL);

        g_signal_connect (view, "button-release-event", G_CALLBACK (mouse_click_cb), view);

        g_signal_connect(G_OBJECT(draw_curse), "draw", 
                G_CALLBACK(on_expose_event), NULL);
        g_timeout_add (20, (GSourceFunc) anim_callback, &draw_curse);
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
