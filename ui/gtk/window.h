#ifndef WINDOW_H
#define WINDOW_H

/**
 * \file window.c
 * \brief Main window creating function.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Main window creating function prototypes.
 *
 */


void create_window(const gchar* title, gint width , gint height, 
		   BrailleMusicEditor *editor);

void window_destroy (GtkWidget *widget, BrailleMusicEditor *editor);

gboolean window_delete_event (GtkWidget *widget, GdkEvent *event, 
			      BrailleMusicEditor *editor);	

#endif
