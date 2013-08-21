/**
 * \file errors.c
 * \brief Error alert functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Error alert functions.
 *
 */


#include <gtk/gtk.h>

/**
 * \fn void open_error(GtkWidget *file_selection, FILE *file, 
 *                     const gchar * message_format, const gchar* path)
 * \brief Display un dialog on file selection window when file == NULL.  
 * \param file_selection The file selection GtkWidget
 * \param file The pointer to the file.
 * \param message_format The message diplayed in the dialog box
 * (must contain only one "%s" with is the path of the file).
 * \param path The path to the file.
 */

void open_error(GtkWidget *file_selection, FILE *file, 
		const gchar * message_format, const gchar* path)
{
    if(file == NULL){
	GtkWidget *dialog;		
	dialog = gtk_message_dialog_new(GTK_WINDOW(file_selection),
					GTK_DIALOG_MODAL, 
					GTK_MESSAGE_ERROR, 
					GTK_BUTTONS_OK, 
					message_format, 
					g_locale_to_utf8(
							 path, -1, NULL, 
							 NULL, NULL));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return;
    }   
}
