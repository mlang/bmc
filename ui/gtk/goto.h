#ifndef GOTO_H
#define GOTO_H

/**
 * \file goto.h
 * \brief Insert cursor moving funtions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Insert cursor moving funtion prototypes.
 *
 */


void goto_next_prev(GtkWidget * widget, BrailleMusicEditor *editor, char n_p); 

void goto_next(GtkWidget * widget, BrailleMusicEditor *editor);

void goto_prev(GtkWidget * widget, BrailleMusicEditor *editor); 

void goto_num(GtkWidget * widget, BrailleMusicEditor *editor); 

void goto_num_(unsigned int n, GtkTextBuffer *buffer);

#endif
