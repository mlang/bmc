/**
 * \file open.c
 * \brief Open file callback.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Open file callback.
 *
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include "BrailleMusicEditor.h"
#include "errors.h"
#include "open.h"
#include "save.h"


/**
 * \fn void open_file(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief That function is called when selecting open file.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 * 
 * This function will show un file selection dialog and
 * check if there is unsaved changes in the editor 
 * calling check_for_save function.
 */
void open_file(GtkWidget *widget, BrailleMusicEditor *editor)
{
    gint resp=check_for_save (editor);
    if (resp == 1){
	save_file(widget, editor);
	open_file_with_selection(editor);
    }			
    else if(resp == 0)
	open_file_with_selection(editor);
    editor->text_changed = FALSE;
}


/**
 * \fn void open_file_with_path(BrailleMusicEditor *editor, const gchar *path)
 * \brief This function opens a file with the path parameter 
 * and show it in editor's textview.
 * \param editor The GUI structure.
 * 
 * This function will check if there is unsaved changes in the editor 
 * calling check_for_save function.
 */
void open_file_with_path(BrailleMusicEditor *editor, const gchar *path) {
    gint resp=check_for_save (editor);
    if (resp == 1){
	save_file(editor->textview, editor);
	open_file_(editor,path);
    }			
    else if(resp == 0)
	open_file_(editor, path);
    editor->text_changed = FALSE;
} 

/**
 * void open_file_with_selection(BrailleMusicEditor *editor)
 * \brief This function actually opens a file in the editor's textview 
 * with a file selection dialog.
 * \param editor The GUI structure.
 */
void open_file_with_selection(BrailleMusicEditor *editor) {
    GtkWidget * file_selection;
    gchar *path;

    //creation of the file selection window
    file_selection = gtk_file_chooser_dialog_new ("Open File", NULL,
						  GTK_FILE_CHOOSER_ACTION_OPEN,
						  GTK_STOCK_CANCEL,
						  GTK_RESPONSE_CANCEL, 
						  GTK_STOCK_OPEN, 
						  GTK_RESPONSE_ACCEPT, NULL);
    
    //banning use of other window
    gtk_window_set_modal(GTK_WINDOW(file_selection), TRUE);
    if(gtk_dialog_run(GTK_DIALOG(file_selection))==GTK_RESPONSE_ACCEPT) {
	// retrival of the file path 
	path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection));
	open_file_(editor, path);
    }
    gtk_widget_destroy(file_selection); 
}


/**
 * \fn void open_file_(BrailleMusicEditor *editor, const gchar *path)
 * \brief This function actually opens a file in the editor's textview.
 * \param editor The GUI structure.
 * \param path the file path.
 */
void open_file_(BrailleMusicEditor *editor, const gchar *path) {
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;
    FILE *file;
    gchar read[1024];
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));

    if(path == NULL)
	return;
    file = fopen(path,"r");
    open_error(editor->window, file, "Can't open the file : \n%s", path);
	
    //save the current file path
    editor->filename = (gchar *)path;
    
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_delete(buffer, &start, &end);
    
    while(fgets(read, 1024, file)){
	gtk_text_buffer_get_end_iter(buffer, &end);
	gtk_text_buffer_insert(buffer, &end, read, -1);
    }
    fclose(file);
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_place_cursor(buffer, &start);
}
