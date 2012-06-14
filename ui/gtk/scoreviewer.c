/**
 * \file scoreviewer.c
 * \brief Scoreviewer creating functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Scoreviewer creating functions.
 *
 */

#include <gtk/gtk.h>
#include <evince-view.h>
#include <stdlib.h>
#include <string.h>
#include "BrailleMusicEditor.h"

/**
 * \fn void external_link_catch(GtkWidget *w, GObject *link) 
 * \brief This function catch the signal when a clic is made on a note.
 * \param w The widget which triggered the call.
 * \param link ev_link_action.
 * \param editor The GUI structure.
 */
void external_link_catch(GtkWidget *widget, GObject *link, BrailleMusicEditor *editor) {
    EvLinkAction *l = EV_LINK_ACTION(link);
    ev_view_copy_link_address(EV_VIEW(widget), l);
    //get line:column link
    const gchar * lc = ev_link_action_get_uri(l);
    int line, column;
    int i = 0;
    char c = ' ', temp[2048];
    while(c != ':' || i >= strlen(lc)) {
	c = lc[i];
	temp[i] = c;
	i++;
    }
    temp[i+1]='\0';
    line = atoi(temp);
    line--;
    column = atoi(lc+i);
    GtkTextIter it;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    if(gtk_text_buffer_get_line_count(buffer) < line ) {
	line = 0;
	column = 0;
    }
	
    gtk_text_buffer_get_iter_at_line(buffer, &it, line);
    while(column >0 && !gtk_text_iter_ends_line(&it)) {
	gtk_text_iter_forward_char(&it);
	column--;
    } 
    gtk_widget_grab_focus(editor->textview);
    gtk_text_buffer_place_cursor(buffer, &it);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(editor->textview), &it, 0.0, FALSE, 0.5, 0.5);
}

/**
 * \fn void show_score(BrailleMusicEditor *editor) 
 * \brief This function create a scoreviewer in a widget.
 * \param editor The GUI structure.
 */
void show_score(BrailleMusicEditor *editor) {
    GError *err = NULL;
    GFile *file;
    gchar *filename = "score.pdf";
    file = g_file_new_for_commandline_arg (filename);
    gchar *uri = g_file_get_uri (file);
    g_object_unref (file);
    EvDocument *doc = ev_document_factory_get_document (uri, &err);
    if(err) {
	g_warning ("Trying to read the score pdf file %s gave an error: %s",
		   uri, err->message);
	if(err)
	    g_error_free (err);
	err = NULL;
    } else {
	EvDocumentModel *docmodel = ev_document_model_new_with_document(EV_DOCUMENT(doc)); 
	ev_view_set_model(EV_VIEW(editor->score_view), EV_DOCUMENT_MODEL(docmodel));
	g_signal_connect(editor->score_view, "external-link", 
			 G_CALLBACK(external_link_catch), editor);
    }
}
