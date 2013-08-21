/**
 * \file edit.c
 * \brief Edit callback functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Edit callback functions.
 *
 */

#include <gtk/gtk.h>
#include "BrailleMusicEditor.h"


/**
 * \fn void on_undo(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_undo(GtkWidget *widget, BrailleMusicEditor *editor)
{
    g_signal_emit_by_name(editor->textview, "undo", NULL);
}

/**
 * \fn void on_redo(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_redo(GtkWidget *widget, BrailleMusicEditor *editor)
{
    g_signal_emit_by_name(editor->textview, "redo", NULL);
}


/**
 * \fn void on_copy(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_copy(GtkWidget *widget, BrailleMusicEditor *editor)
{
    g_signal_emit_by_name(editor->textview, "copy-clipboard", NULL);
}

/**
 * \fn void on_cut(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_cut(GtkWidget *widget, BrailleMusicEditor *editor)
{
    g_signal_emit_by_name(editor->textview, "cut-clipboard", NULL);
}

/**
 * \fn void on_paste(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_paste(GtkWidget *widget, BrailleMusicEditor *editor)
{
    g_signal_emit_by_name(editor->textview, "paste-clipboard", NULL);
}

/**
 * \fn void on_select_all(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_select_all(GtkWidget *widget, BrailleMusicEditor *editor)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_start_iter(buffer,&start);
    gtk_text_buffer_get_end_iter(buffer,&end);
    gtk_text_buffer_create_mark(buffer, "selection_bound", &start, FALSE);
    gtk_text_buffer_move_mark_by_name(buffer, "insert", &end);
}


/**
 * \fn void text_changed(GtkWidget *widget, BrailleMusicEditor *editor)
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void on_text_changed(GtkWidget *widget, BrailleMusicEditor *editor)
{
    editor->text_changed = TRUE;
}

