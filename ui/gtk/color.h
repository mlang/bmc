#ifndef COLOR_H
#define COLOR_H

/**
 * \file color.h
 * \brief Braille Music lexical coloration.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Lexical Highlighting functions prototypes
 *
 */

void lexical_coloration(GtkWidget *widget, BrailleMusicEditor *editor);

void lexical_coloration_off(BrailleMusicEditor *editor); 

void coloration_update(GtkWidget *widget, BrailleMusicEditor *editor);

void init_braille_table();

void init_colors();

void set_tags(GtkTextBuffer *buffer);

void color(BrailleMusicEditor *editor);

void lexical_coloration_off(BrailleMusicEditor *editor); 

void color_options(GtkWidget *widget, BrailleMusicEditor *editor);

void color_selection(GtkWidget *widget, BrailleMusicEditor *editor);

#endif
