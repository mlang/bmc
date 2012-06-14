/**
 * \file new.c
 * \brief New file callback.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * New file callback.
 *
 */

#include <gtk/gtk.h>
#include "BrailleMusicEditor.h"
#include "new.h"
#include "save.h"


/**
 * \fn void new_file(GtkWidget * widget, BrailleMusicEditor *editor)
 * \brief The function called when creating a new file.
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 *
 * The function will check and see if there is data on the editor 
 * and ask the user to save or not by calling check_for_save.
 */

void new_file(GtkWidget * widget, BrailleMusicEditor *editor)
{
    gint resp=check_for_save (editor);
    if (resp == 1){
	save_file(widget, editor);
	new_file_(editor);
    }			
    else if(resp == 0)
	new_file_(editor);    
}


/** 
 * \fn void new_file_(BrailleMusicEditor *editor)
 * \brief This function actually opens a new file.
 * \param editor The GUI structure.
 */
void new_file_(BrailleMusicEditor *editor)
{
    GtkTextBuffer *buffer;
    editor->filename = NULL;
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
    gtk_text_buffer_set_text (buffer, "", -1);
    gtk_text_buffer_set_modified (buffer, FALSE);	
}
