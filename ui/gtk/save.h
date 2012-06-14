#ifndef SAVE_H
#define SAVE_H

/**
 * \file save.h
 * \brief Save file callback.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Save file callback prototypes.
 *
 */

void save_file(GtkWidget *widget, BrailleMusicEditor *editor);

void save_file_as(GtkWidget *widget, BrailleMusicEditor *editor);

void write_file(gchar *path,GtkTextBuffer *buffer, 
		GtkWidget *file_selection, BrailleMusicEditor *editor);

gint check_for_save (BrailleMusicEditor *editor);

#endif
