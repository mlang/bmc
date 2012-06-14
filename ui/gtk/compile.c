/**
 * \file compile.c
 * \brief Compile the current file.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Compile the current file by calling BMC.
 *
 */

#include <gtk/gtk.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "BrailleMusicEditor.h"
#include "scoreviewer.h"
#include "save.h"
#include "tops.h"
#include <unistd.h>

#define BMC_DIR "../../bmc"
#define BMC_EXE "./bmc"


/**
 * \fn void compile(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief Compile callback function
 * \param widget The widget which triggered the call.
 * \param editor The GUI structure.
 *
 * This function triggers the compilation of the current file by using BMC.
 * it save the current opened file.
 */
void compile(GtkWidget *widget, BrailleMusicEditor *editor) {
    

    GPid pid;
    GtkTextBuffer *buffer;
    char *argv[2];
    gchar cmd[] = BMC_EXE;
    int standard_output, standard_error; 
    gchar buf[1024];
    gint chars_read = 1024;
    GtkTextIter iter;
    int fd, ly_fd;
    
    unlink("score.ly");
    unlink("score.ps");
    unlink("score.pdf");
    unlink("score.midi");
    save_file(widget, editor);
    
    fd = open(editor->filename, O_RDONLY);
    ly_fd = open("score.ly", O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); 
    
    dup2(fd, STDIN_FILENO);
    argv[0]=cmd;
    argv[1]=NULL;
    g_spawn_async_with_pipes( BMC_DIR , // const gchar *working_directory
    			      argv,        // gchar **argv
    			      NULL,        // gchar **envp
    			      G_SPAWN_SEARCH_PATH | 
			      G_SPAWN_CHILD_INHERITS_STDIN ,       // GSpawnFlags flags
    			      NULL,       // GSpawnChildSetupFunc child_setup
    			      NULL,       // gpointer user_data
    			      &pid,       // GPid *child_pid
    			      NULL,       // gint *standard_input
    			      &standard_output,   // gint *standard_output
    			      &standard_error,    // gint *standard_error
    			      NULL);    // GError **error

    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->error_view));
    gtk_text_buffer_get_end_iter(buffer, &iter);
    
    while (chars_read > 0){
	chars_read = read(standard_error, buf, 1024);
	gtk_text_buffer_insert (buffer, &iter, buf, chars_read);
	
    }
    chars_read = 1024;
    while (chars_read > 0){
	chars_read = read(standard_output, buf, 1024);
	write(ly_fd, buf, chars_read);
    }
    tops("score");
    close(fd);
    close(ly_fd);
    close(standard_error);
    close(standard_output);
    show_score(editor);
}
