#include "dronesimulation-window.h"

gboolean
draw_center (ClutterCanvas *canvas,
    cairo_t *cr,
    int width,
    int height)
{
  cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
  cairo_paint(cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

  /* Draw the circle */
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_arc (cr, MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, MARKER_SIZE / 2.0, 0, 2 * M_PI);
  cairo_close_path (cr);

  /* Fill the circle */
  cairo_set_source_rgba (cr, 0.1, 0.1, 0.9, 1.0);
  cairo_fill (cr);
  
  return TRUE;
}


gboolean
draw_circle (ClutterCanvas *canvas,
    cairo_t *cr,
    int width,
    int height)
{
   /* Draw the circle */
  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_arc (cr, MARKER_SIZE, MARKER_SIZE, 0.9 * MARKER_SIZE, 0, 2 * M_PI);
  cairo_close_path (cr);

  /* Stroke the circle */
  cairo_set_line_width (cr, MARKER_SIZE/5.0);
  cairo_set_source_rgba (cr, 0.1, 0.1, 0.7, 1.0);
  cairo_stroke (cr);

  return TRUE;
}


/* The marker is drawn with cairo.  It is composed of 1 static filled circle
 * and 1 stroked circle animated as an echo.
 */
ClutterActor *
create_marker ()
{
  ClutterActor *marker;
  ClutterActor *bg;
  ClutterContent *canvas;
  ClutterTransition *transition;

  /* Create the marker */
  marker = champlain_custom_marker_new ();

  /* Static filled circle ----------------------------------------------- */
  canvas = clutter_canvas_new ();
  clutter_canvas_set_size (CLUTTER_CANVAS (canvas), MARKER_SIZE, MARKER_SIZE);
  g_signal_connect (canvas, "draw", G_CALLBACK (draw_center), NULL);

  bg = clutter_actor_new ();
  clutter_actor_set_size (bg, MARKER_SIZE, MARKER_SIZE);
  clutter_actor_set_content (bg, canvas);
  clutter_content_invalidate (canvas);
  g_object_unref (canvas);

  /* Add the circle to the marker */
  clutter_actor_add_child (marker, bg);
  clutter_actor_set_position (bg,  -0.5*MARKER_SIZE,  -0.5*MARKER_SIZE);

  /* Echo circle -------------------------------------------------------- */
  canvas = clutter_canvas_new ();
  clutter_canvas_set_size (CLUTTER_CANVAS (canvas), 2 * MARKER_SIZE, 2 * MARKER_SIZE);
  g_signal_connect (canvas, "draw", G_CALLBACK (draw_circle), NULL);

  bg = clutter_actor_new ();
  clutter_actor_set_size (bg, 2.0 * MARKER_SIZE, 2.0 * MARKER_SIZE);
  clutter_actor_set_content (bg, canvas);
  clutter_content_invalidate (canvas);
  g_object_unref (canvas);

  /* Add the circle to the marker */
  clutter_actor_add_child (marker, bg);
  clutter_actor_set_pivot_point (bg, 0.5, 0.5);
  clutter_actor_set_position (bg, -MARKER_SIZE, -MARKER_SIZE);

  transition = clutter_property_transition_new ("opacity");
  clutter_actor_set_easing_mode (bg, CLUTTER_EASE_OUT_SINE);
  clutter_timeline_set_duration (CLUTTER_TIMELINE (transition), 1000);
  clutter_timeline_set_repeat_count (CLUTTER_TIMELINE (transition), -1);
  clutter_transition_set_from (transition, G_TYPE_UINT, 255);
  clutter_transition_set_to (transition, G_TYPE_UINT, 0);
  clutter_actor_add_transition (bg, "animate-opacity", transition);

  transition = clutter_property_transition_new ("scale-x");
  clutter_actor_set_easing_mode (bg, CLUTTER_EASE_OUT_SINE);
  clutter_timeline_set_duration (CLUTTER_TIMELINE (transition), 1000);
  clutter_timeline_set_repeat_count (CLUTTER_TIMELINE (transition), -1);
  clutter_transition_set_from (transition, G_TYPE_FLOAT, 0.5);
  clutter_transition_set_to (transition, G_TYPE_FLOAT, 2.0);
  clutter_actor_add_transition (bg, "animate-scale-x", transition);

  transition = clutter_property_transition_new ("scale-y");
  clutter_actor_set_easing_mode (bg, CLUTTER_EASE_OUT_SINE);
  clutter_timeline_set_duration (CLUTTER_TIMELINE (transition), 1000);
  clutter_timeline_set_repeat_count (CLUTTER_TIMELINE (transition), -1);
  clutter_transition_set_from (transition, G_TYPE_FLOAT, 0.5);
  clutter_transition_set_to (transition, G_TYPE_FLOAT, 2.0);
  clutter_actor_add_transition (bg, "animate-scale-y", transition);

  return marker;
}


int clutter_actor_set_center_pos(ClutterActor *actor, gfloat x, gfloat y)
{
    gfloat width, height;
    clutter_actor_get_size(actor, &width, &height);
    clutter_actor_set_x(actor, x-0.5*width);
    clutter_actor_set_y(actor, y-0.5*height);
    return 0;
}

int test_marker(int argc, char *argv[])
{
    printf("First step;\n");
    ClutterActor *marker, *stage;

    if (clutter_init (&argc, &argv) != CLUTTER_INIT_SUCCESS)
        return 1;

    stage = clutter_stage_new ();
    gfloat width, height;
    clutter_actor_get_size(stage, &width, &height);
    printf("Stage Size: %f,%f\n",width,height);


    g_signal_connect (stage, "destroy", G_CALLBACK (clutter_main_quit), NULL);


    marker = create_marker ();
    gfloat mwidth, mheight;
    clutter_actor_get_size(marker, &mwidth, &mheight);
    printf("Marker Size: %f,%f\n",mwidth,mheight);
    clutter_actor_set_center_pos(marker, width*0.5, height*0.5);

    clutter_actor_add_child (stage, marker);

    clutter_actor_show (stage);
    clutter_main ();

    return 0;
}
