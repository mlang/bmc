/**
 * \file main.c
 * \brief GUI main function.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * GUI main function.
 *
 */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <evince-view.h>
#include "BrailleMusicEditor.h"
#include "window.h"
#include "toolbar.h"
#include "menubar.h"
#include "color.h"
#include "goto.h"
#include "scoreviewer.h"
#include "edit.h"



/**
 * \fn int main(int argc, char **argv)
 * \brief GUI main function.
 * \param argc Number of parameters.
 * \param argv Array of parameters.
 * This function creates all the widgets of the structure BrailleMusicEditor
 * and run le GUI loop.
 */

int main(int argc, char **argv)
{
    BrailleMusicEditor *editor;

    // allocate the memory needed by our BrailleMusicEditor struct 
    editor = g_slice_new (BrailleMusicEditor);
 
    //initialize GTK+ libraries
    gtk_init(&argc, &argv);
    ev_init();

    // Creation of the main window 
    create_window("BMC",600,400, editor);
	
    editor->vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
        gtk_container_add(GTK_CONTAINER(editor->window), editor->vbox); 
	
    //Creation of the menubar
    create_menubar(editor);
    gtk_box_pack_start(GTK_BOX(editor->vbox), editor->menubar, FALSE, FALSE, 0);

    // Creation of the toolbar
    create_toolbar(editor);
    gtk_box_pack_start(GTK_BOX(editor->vbox), editor->toolbar, FALSE, FALSE, 2);
	
    //Creation of the two text views with scrollbars

    editor->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(editor->vbox), editor->hbox, TRUE, TRUE, 0);
	
    editor->edit_scrollbar = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor->edit_scrollbar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(editor->hbox),editor->edit_scrollbar, TRUE, TRUE, 5);

    editor->textview=gtk_source_view_new(); 
    //show line number
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(editor->textview), TRUE);
    g_signal_connect(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)), "changed", G_CALLBACK(on_text_changed), editor);
    gtk_container_add(GTK_CONTAINER(editor->edit_scrollbar), editor->textview);
    
    //lexical coloration auto update
    g_signal_connect(gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)), "changed", G_CALLBACK(coloration_update), editor);

    //score view
    editor->score_scrollbar = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(editor->hbox),editor->score_scrollbar, TRUE, TRUE, 5);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor->score_scrollbar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    editor->score_view = ev_view_new();
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(editor->score_scrollbar), editor->score_view);	
    
    
    gtk_widget_grab_focus(editor->textview);
    
    //Creation of the message error views with scrollbar
    editor->error_scrollbar = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(editor->vbox),editor->error_scrollbar, FALSE, TRUE, 0);
    gtk_widget_set_size_request (editor->error_scrollbar, -1, 100);
    gtk_container_set_border_width(GTK_CONTAINER(editor->error_scrollbar), 5);
    editor->error_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(editor->error_view), FALSE);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(editor->error_scrollbar), editor->error_view);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor->error_scrollbar), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    
    // show the window
    gtk_widget_show_all(editor->window);

    // enter GTK+ main loop
    gtk_main();

    // free memory we allocated for BrailleMusicEditor struct 
    g_slice_free (BrailleMusicEditor, editor);

    return EXIT_SUCCESS;
}


