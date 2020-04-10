#include <cairo.h>
#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

double start = 5;
double end = 110;
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
        cairo_move_to(data->cr, data->posx, data->posy);
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

static void do_drawing(cairo_t *cr, GtkWidget *widget)
{         
    GtkWidget *win = gtk_widget_get_toplevel(widget);



    gint width, height;
    gtk_window_get_size(GTK_WINDOW(win), &width, &height);

    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_move_to(cr, 0, 0);
    cairo_rel_line_to(cr, width, 0);
    cairo_rel_line_to(cr, 0, height);
    cairo_rel_line_to(cr, -width, height);
    cairo_close_path(cr);
    cairo_fill(cr);

    double radius = 0.35 * height;
    char string[255];
    cairo_stroke(cr);

    cairo_text_extents_t te;
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // Draw 
    // Circle plate
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, width * 0.5, height * 0.5, radius, 0, 2*M_PI);
    cairo_stroke(cr);

    cairo_set_font_size(cr, width/100.0);

    for (int i = 0; i < 8; ++i) {
        cairo_move_to(cr, 0.5 * width, 0.5 * height);
        cairo_rotate(cr, i*0.25*M_PI);
        cairo_rel_move_to(cr, 0.9*radius, 0);
        cairo_rel_line_to(cr, 0.1*radius, 0);
        sprintf(string, "%.0f",i*0.25*360);
        cairo_rotate(cr, 0.5*M_PI);
        cairo_text_extents(cr, string, &te);
        cairo_rel_move_to(cr, - te.width / 2.0 , te.y_bearing);
        cairo_show_text(cr,string);
        cairo_rotate(cr, -0.5*M_PI);
        cairo_rotate(cr, i*-0.25*M_PI);
    }
    cairo_stroke(cr);

    // Draw needle
    double theta = THETA;
    cairo_set_source_rgba(cr, 0.5, 0.4, 0.4, 1.0);
    cairo_set_line_width(cr, 4);

    cairo_move_to(cr, width * .5, height * .5);
    cairo_rel_line_to(cr, 1.0*radius*cos(theta), 1.0*radius*sin(theta));
    cairo_stroke(cr);

    cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    cairo_move_to(cr, width * .5, height * .5);
    cairo_arc(cr, width * 0.5, height * 0.5, 5, 0, 2*M_PI);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);

    coord mcoord;
    mcoord.cr = cr;

    mcoord.posx = 0.1*width;
    mcoord.posy = 0.9*height;

    mcoord.llength = 0.8* width;
    mcoord.start = start;
    mcoord.end = end;
    mcoord.tics = 10;
    mcoord.tics_length = 9;
    mcoord.ori = 'x';
    mcoord.tics_ori = 1.0;
    mcoord.tics_level = 3;
    mcoord.tics_ori = TICS_ORI_UP;
    mcoord.linewidth = width / 500.0;
    mcoord.fontsize = mcoord.linewidth * 5;
    draw_coord(&mcoord);

    mcoord.ori = 'y';
    mcoord.llength = 0.8* height;
    mcoord.tics_ori = TICS_ORI_RIGHT;
    draw_coord(&mcoord);

    mcoord.posx = 0.9*width;
    mcoord.posy = 0.9*height;
    mcoord.ori = 'y';
    mcoord.tics_ori = TICS_ORI_LEFT;
    draw_coord(&mcoord);

    mcoord.posx = 0.1*width;
    mcoord.posy = 0.1*height;
    mcoord.llength = 0.8* width;
    mcoord.ori = 'x';
    mcoord.tics_ori = TICS_ORI_DOWN;
    draw_coord(&mcoord);



    cairo_stroke(cr);
}

static gboolean
anim_callback (void *ptr)
{
    THETA += 0.01 * M_PI;
    return TRUE;
}

int main(int argc, char *argv[])
{
  GtkWidget *window;
  GtkWidget *darea;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  darea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER (window), darea);

  g_signal_connect(G_OBJECT(darea), "draw", 
      G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(G_OBJECT(window), "destroy",
      G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 960, 960); 
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  gtk_window_set_title(GTK_WINDOW(window), "Donut");
  g_timeout_add (20, (GSourceFunc) anim_callback, &darea);
  
  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
