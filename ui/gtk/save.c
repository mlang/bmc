/**
 * \file save.c
 * \brief Save file callback.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Save file callback.
 *
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include "BrailleMusicEditor.h"
#include "save.h"
#include "errors.h"

/**
 * \fn void save_file(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief This function is called when selecting save file.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 * 
 * This function displays the window where one can chose 
 * in which file the data has to be saved.
 */
void save_file(GtkWidget *widget, BrailleMusicEditor *editor)
{
    GtkTextBuffer *buffer;
    GtkWidget *file_selection;
    gchar *path;
	
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview));
	
    if(editor->filename!=NULL)
	write_file(editor->filename,buffer, NULL, editor);
    else{
	//creation of the file selection window
	file_selection = gtk_file_chooser_dialog_new ("Save file", NULL, 
						      GTK_FILE_CHOOSER_ACTION_SAVE,
						      GTK_STOCK_CANCEL, 
						      GTK_RESPONSE_CANCEL, 
						      GTK_STOCK_SAVE, 
						      GTK_RESPONSE_ACCEPT, NULL);
	//banning use of other window
	gtk_window_set_modal(GTK_WINDOW(file_selection), TRUE);

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(file_selection), 
						       TRUE);
	
	if(gtk_dialog_run(GTK_DIALOG(file_selection))==GTK_RESPONSE_ACCEPT){
	    path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_selection));
	    write_file(path,buffer, file_selection, editor);
	}
	gtk_widget_destroy(file_selection);
    }
    editor->text_changed=FALSE;
}

/** 
 * \fn void save_file_as(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief This function is called when selecting save file as.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 */	
void save_file_as(GtkWidget *widget, BrailleMusicEditor *editor)
{
    editor->filename=NULL;
    save_file(widget, editor);
}


/**
 * \fn void write_file(gchar *path,GtkTextBuffer *buffer, 
 * GtkWidget *file_selection, BrailleMusicEditor *editor)
 * \brief Once save_file is called, the function calls write_file 
 * to save the content of the editor into a file.
 * \param path The file in which to save the data.
 * \param buffer The buffer where the data is.
 * \param file_selection file selection widget.
 * \param editor The GUI structure.
 */
void write_file(gchar *path,GtkTextBuffer *buffer, 
		GtkWidget *file_selection, BrailleMusicEditor *editor)
{
    FILE *file;
    GtkTextIter start;
    GtkTextIter end;
	
    file = fopen(path,"w+");
    open_error(file_selection, file, "Can't save in the file : \n%s", path);
	
    //save the current file path
    editor->filename=path;
	
    gtk_text_buffer_get_start_iter(buffer,&start);
    gtk_text_buffer_get_end_iter(buffer,&end);
	
    const gchar *write=gtk_text_buffer_get_text(buffer,&start, &end,TRUE);
	
    fputs(write, file);	
    fclose(file);
}


/**
 * \fn  gint check_for_save (BrailleMusicEditor *editor)
 * \brief This functions returns the boolean that represents 
 * whether the user wants to save changes before exiting the program.
 * \param editor The structure that holds the data.
 * \return 0 means no, 1 means yes, 2 means cancel.
 */

gint check_for_save (BrailleMusicEditor *editor)
{
    int ret = 0;
    //0 means no, 1 means yes, 2 means cancel 

    if (editor->text_changed == TRUE){
	// we need to prompt for save 
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new(GTK_WINDOW(editor->window), 
					GTK_DIALOG_MODAL|
					GTK_DIALOG_DESTROY_WITH_PARENT, 
					GTK_MESSAGE_QUESTION, 
					GTK_BUTTONS_YES_NO,
					"Do you want to save the changes you have made?");
	gtk_window_set_title (GTK_WINDOW (dialog), "Save?");
	
	gint resp=gtk_dialog_run (GTK_DIALOG (dialog));
	if ( resp == GTK_RESPONSE_YES)
	    ret = 1;
	else if( resp == GTK_RESPONSE_NO)
	    ret = 0;
	else
	    ret = 2;
	gtk_widget_destroy (dialog);     
    }     	
    return ret;
}
