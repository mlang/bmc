#ifndef EDIT_H
#define EDIT_H

/**
 * \file edit.h
 * \brief Edit callback functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Edit callback function prototypes.
 *
 */


void on_undo(GtkWidget *widget, BrailleMusicEditor *editor);

void on_redo(GtkWidget *widget, BrailleMusicEditor *editor);

void on_copy(GtkWidget *widget, BrailleMusicEditor *editor);

void on_cut(GtkWidget *widget, BrailleMusicEditor *editor);

void on_paste(GtkWidget *widget, BrailleMusicEditor *editor);

void on_select(GtkWidget *widget, BrailleMusicEditor *editor);

void on_select_all(GtkWidget *widget, BrailleMusicEditor *editor);

void on_text_changed(GtkWidget *widget, BrailleMusicEditor *editor);
#endif
