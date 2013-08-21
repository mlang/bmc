#ifndef OPEN_H
#define OPEN_H

/**
 * \file open.h
 * \brief Open file callback.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Open file callback prototype.
 *
 */

void open_file(GtkWidget *widget, BrailleMusicEditor *editor);

void open_file_with_path(BrailleMusicEditor *editor, const gchar *path);

void open_file_with_selection(BrailleMusicEditor *editor);

void open_file_(BrailleMusicEditor *editor, const gchar *path); 
#endif
