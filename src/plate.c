#include <stdio.h>
#include <math.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

#define WIDTH 640
#define HEIGHT 480

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
} coord;

enum {
    TICS_ORI_RIGHT = 1,
    TICS_ORI_LEFT = -1, 
    TICS_ORI_UP = -1,
    TICS_ORI_DOWN = 1
};

int coord_set_tics_ori(coord *data, int tics_ori)
{
    if( tics_ori == TICS_ORI_LEFT )
        data->tics_ori = -1.0;
    else if( tics_ori == TICS_ORI_RIGHT )
        data->tics_ori = 1.0;
    else if( tics_ori == TICS_ORI_UP )
        data->tics_ori = -1.0;
    else if( tics_ori == TICS_ORI_DOWN )
        data->tics_ori = 1.0;
    return 0;
}

int draw_coord(coord *data)
{

    cairo_select_font_face(data->cr, "Purisa",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(data->cr, data->tics_length * 0.7);

    char string[255];

    double ini_linew = cairo_get_line_width(data->cr);
    double linewidth = HEIGHT / 300.0;
    cairo_set_line_width(data->cr, linewidth);

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
                cairo_set_line_width(data->cr, linewidth * pow(0.7,i));
                cairo_rel_line_to(data->cr, 0, data->tics_ori * data->tics_length * pow(0.7,i));
                if ( i == 1 ){
                    if (data->tics_ori == TICS_ORI_DOWN)
                        text_align = fe.descent;
                    else 
                        text_align =  fe.height;
                    printf("tics val: %f\n",(tics_pos - data->posx)/step_length + data->start);
                    cairo_rel_move_to(data->cr, -1 * data->tics_length * 0.7, data->tics_ori * (data->tics_length *  -1 * pow(0.7,i) - text_align));
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
                cairo_set_line_width(data->cr, linewidth * pow(0.7,i));
                cairo_rel_line_to(data->cr, data->tics_ori * data->tics_length * pow(0.7,i), 0);
                if ( i == 1 ){
                    sprintf(string, "%.2f", -1 * (tics_pos - data->posy)/step_length + data->start);
                    if(data->tics_ori == TICS_ORI_LEFT){
                        cairo_text_extents(data->cr, "a", &te);
                        text_align = - te.width;
                        printf("text_align:%f\n",text_align);
                    }else if(data->tics_ori == TICS_ORI_RIGHT){
                        cairo_text_extents(data->cr, string, &te);
                        text_align = - te.x_bearing - te.width - te.height;
                        printf("text_align:%f\n",text_align);
                    }else{continue;}
                    cairo_rel_move_to(data->cr, data->tics_ori * ( -1 * data->tics_length * pow(0.7,i) + text_align)  , fe.height/2.0 );
                    cairo_show_text(data->cr, string);
                }
            }
            cairo_stroke(data->cr);

        }
    }

    cairo_set_line_width(data->cr, ini_linew);
    return 0;
}

int main(int argc, char *argv[])
{
    cairo_t *cr;
    cairo_surface_t *surface;
    surface = cairo_pdf_surface_create("plate.pdf", WIDTH, HEIGHT);
    cr = cairo_create(surface);

    cairo_set_source_rgba(cr, 0., 0., 0., 1.0);
    /* cairo_set_line_width(cr, 2); */

    double radius = 0.4 * HEIGHT;
    char string[255];
    cairo_stroke(cr);

    cairo_text_extents_t te;
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    // Draw 
    // Circle plate
    cairo_set_line_width(cr, 2.0);
    cairo_arc(cr, WIDTH * 0.5, HEIGHT * 0.5, radius, 0, 2*M_PI);

    for (int i = 0; i < 8; ++i) {
        cairo_move_to(cr, 0.5 * WIDTH, 0.5 * HEIGHT);
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
    double theta = 0.6 * M_PI;
    cairo_set_source_rgba(cr, 0.5, 0.4, 0.4, 1.0);
    cairo_set_line_width(cr, 4);

    cairo_move_to(cr, WIDTH * .5, HEIGHT * .5);
    cairo_rel_line_to(cr, 0.6*radius*cos(theta), 0.6*radius*sin(theta));
    cairo_stroke(cr);

    cairo_set_source_rgba(cr, 0, 0, 0, 1.0);
    cairo_move_to(cr, WIDTH * .5, HEIGHT * .5);
    cairo_arc(cr, WIDTH * 0.5, HEIGHT * 0.5, 5, 0, 2*M_PI);
    cairo_close_path(cr);
    cairo_fill(cr);

    cairo_stroke(cr);


    // Draw outer coord;
    coord mcoord;
    mcoord.cr = cr;

    mcoord.posx = 0.5*WIDTH - 0.45 * HEIGHT;
    mcoord.posy = 0.95*HEIGHT;

    mcoord.llength = 0.9*HEIGHT;
    mcoord.start = -25;
    mcoord.end = 55.5;
    mcoord.tics = 10;
    mcoord.tics_length = 9;
    mcoord.ori = 'x';
    mcoord.tics_ori = 1.0;
    mcoord.tics_level = 3;
    /* coord_set_tics_ori(&mcoord, TICS_ORI_UP); */
    mcoord.tics_ori = TICS_ORI_UP;
    draw_coord(&mcoord);

    mcoord.ori = 'y';
    /* coord_set_tics_ori(&mcoord, TICS_ORI_LEFT); */
    mcoord.tics_ori = TICS_ORI_RIGHT;
    draw_coord(&mcoord);

    mcoord.posx = 0.5*WIDTH + 0.45 * HEIGHT;
    mcoord.posy = 0.95*HEIGHT;
    mcoord.ori = 'y';
    mcoord.tics_ori = TICS_ORI_LEFT;
    draw_coord(&mcoord);

    mcoord.posx = 0.5*WIDTH - 0.45 * HEIGHT;
    mcoord.posy = 0.05*HEIGHT;
    mcoord.ori = 'x';
    mcoord.tics_ori = TICS_ORI_DOWN;
    draw_coord(&mcoord);

    printf("%f\n",pow(10.0, 2));

    cairo_surface_show_page(surface);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
