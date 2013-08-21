/**
 * \file goto.c
 * \brief Insert cursor moving funtions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Insert cursor moving funtions.
 *
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include "BrailleMusicEditor.h"
#include "goto.h"


/**
 * \fn void goto_next_prev(GtkWidget * widget, BrailleMusicEditor *editor, char n_p)
 * \brief This funtion moves the cursor to the next/previous measure
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 * \param n_p 'n' for next, 'p' for previous 
 */
void goto_next_prev(GtkWidget * widget, BrailleMusicEditor *editor, char n_p) {
    GtkTextIter start_find, end_find;
    GtkTextIter start_match, end_match;
    GtkTextMark *cursor;
    GtkTextBuffer * buffer; 
    gchar bar_line[] = "⠀"; //Warning : Braille character
    
    // Get the textview buffer. 
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
    // Get the mark at cursor. 
    cursor = gtk_text_buffer_get_mark (buffer, "insert");
    // Get the iter at cursor. 
    gtk_text_buffer_get_iter_at_mark (buffer, &start_find, cursor);
    
    gtk_text_buffer_get_end_iter(buffer, &end_find);
    
    gboolean go;
    if(n_p == 'n')
	go = gtk_text_iter_forward_search(&start_find, bar_line, 
					    GTK_TEXT_SEARCH_TEXT_ONLY | 
					    GTK_TEXT_SEARCH_VISIBLE_ONLY, 
					    &start_match, &end_match, NULL);
    else if(n_p == 'p') {
	gtk_text_iter_backward_cursor_position(&start_find);
	go = gtk_text_iter_backward_search(&start_find, bar_line, 
					   GTK_TEXT_SEARCH_TEXT_ONLY | 
					   GTK_TEXT_SEARCH_VISIBLE_ONLY, 
					   &start_match, &end_match, NULL);
    }
    if (go) {
	gtk_text_buffer_place_cursor(buffer, &end_match); 
	int offset = gtk_text_iter_get_offset(&end_match);
	gtk_text_buffer_get_iter_at_offset(buffer, &start_find, offset);
    }
}


/**
 * \fn void goto_next(GtkWidget * widget, BrailleMusicEditor *editor) 
 * \brief This funtion moves the cursor to the next measure
 * by calling goto_next_prev with n_p='n'.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void goto_next(GtkWidget * widget, BrailleMusicEditor *editor) {
    goto_next_prev(widget, editor, 'n');
}

/**
 * \fn void goto_prev(GtkWidget * widget, BrailleMusicEditor *editor) 
 * \brief This funtion moves the cursor to the previous measure
 * by calling goto_next_prev with n_p='p'.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void goto_prev(GtkWidget * widget, BrailleMusicEditor *editor) {
    goto_next_prev(widget, editor, 'p');
}

/**
 * \fn void goto_num(GtkWidget * widget, BrailleMusicEditor *editor)  
 * \brief This funtion moves the cursor to the nth measure
 * by calling the auxiliary function goto_num_().
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */
void goto_num(GtkWidget * widget, BrailleMusicEditor *editor) {	
    GtkWidget* dialog;
    GtkWidget* entry;
    const gchar* snum;
    unsigned int n;
    // Creation of a dialog window
    dialog = gtk_dialog_new_with_buttons("Go to the measure n°?",
					 GTK_WINDOW(editor->window),
					 GTK_DIALOG_MODAL,
					 GTK_STOCK_CANCEL,
					 GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,
					 GTK_RESPONSE_OK,
					 NULL);

    // Creation of the entry zone
    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Enter the measure number");
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(box), entry, TRUE, FALSE, 0);
 
    gtk_widget_show_all(box);

    // run the dialog window
    switch (gtk_dialog_run(GTK_DIALOG(dialog)))
	{
        case GTK_RESPONSE_OK:
            snum = gtk_entry_get_text(GTK_ENTRY(entry));
	    n = atoi(snum);
	    goto_num_(n,gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)));
	    break;
        case GTK_RESPONSE_CANCEL:
        case GTK_RESPONSE_NONE:
        default:
            break;
	}

    // Destruction of the dialog window
    gtk_widget_destroy(dialog); 
	
}

/**
 * \fn void goto_num_(unsigned int n, GtkTextBuffer *buffer)  
 * \brief This funtion moves the cursor to the nth measure in GtkTextBuffer.
 * \param n The measure's number.
 * \param buffer The GtkTextBuffer.
 */
void goto_num_(unsigned int n, GtkTextBuffer *buffer) {
    GtkTextIter start_find, end_find;
    GtkTextIter start_match, end_match;
    unsigned int num = 0;
    gchar bar_line[] = "⠀"; //Warning : Braille character
    
    gtk_text_buffer_get_start_iter(buffer, &start_find);
    gtk_text_buffer_get_end_iter(buffer, &end_find);
    
    while(num < n &&
	  gtk_text_iter_forward_search(&start_find, bar_line, 
				       GTK_TEXT_SEARCH_TEXT_ONLY | 
				       GTK_TEXT_SEARCH_VISIBLE_ONLY, 
				       &start_match, &end_match, NULL)) {
	
	
	int offset = gtk_text_iter_get_offset(&end_match);
	gtk_text_buffer_get_iter_at_offset(buffer, &start_find, offset);
	num++;
    }
    if (num > 0)
	gtk_text_buffer_place_cursor(buffer, &end_match); 
} 
