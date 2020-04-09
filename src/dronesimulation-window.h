#include <champlain/champlain.h>
#include <gtk/gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#include <math.h>

#define MARKER_SIZE 10

gboolean draw_center(ClutterCanvas *canvas, cairo_t *cr, int width, int height);

gboolean draw_circle(ClutterCanvas *canvas, cairo_t *cr, int width, int height);

ClutterActor *create_marker ();

int clutter_actor_set_center_pos(ClutterActor *actor, gfloat x, gfloat y);

int test_marker(int argc, char *argv[]);
