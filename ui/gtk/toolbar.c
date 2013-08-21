/**
 * \file toolbar.c
 * \brief Toolbar creating function.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Toolbar creating function.
 *
 */

#include <gtk/gtk.h>
#include "BrailleMusicEditor.h"
#include "new.h"
#include "open.h"
#include "save.h"
#include "edit.h"
#include "window.h"
#include "color.h"
#include "player.h"
#include "compile.h"



/**
 * \fn void create_toolbar(BrailleMusicEditor *editor)
 * \brief That function creates the toolbar of the window 
 * (the bar with the buttons open, copy, paste ...)
 * \param editor The structure in which to create the toolbar.
 */

void create_toolbar(BrailleMusicEditor *editor)
{	
    editor->toolbar=gtk_toolbar_new();
	
    /* Addition of buttons in the toolbar */
    GtkToolItem *new, *open, *save, *sep, *undo, *redo, 
	*cut, *copy, *paste, *sep1, *sep2, *sep3, *play, 
	*pause, *stop, *quit, *comp;
    
    new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), new, -1);
    gtk_tool_item_set_tooltip_text(new, "Create a new file");
    g_signal_connect(G_OBJECT(new),"clicked",G_CALLBACK(new_file),editor);
   
    open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), open, -1);
    g_signal_connect(G_OBJECT(open),"clicked",G_CALLBACK(open_file),editor);
    gtk_tool_item_set_tooltip_text(open, "Open a file");

    save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), save, -1);
    gtk_tool_item_set_tooltip_text(save, "Save the current file");
    g_signal_connect(G_OBJECT(save),"clicked",G_CALLBACK(save_file),editor);

    sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), sep, -1); 

    undo = gtk_tool_button_new_from_stock(GTK_STOCK_UNDO);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), undo, -1);
    gtk_tool_item_set_tooltip_text(undo, "Undo");
    g_signal_connect(G_OBJECT(undo),"clicked",G_CALLBACK(on_undo),editor);

    redo = gtk_tool_button_new_from_stock(GTK_STOCK_REDO);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), redo, -1);
    gtk_tool_item_set_tooltip_text(redo, "Redo");
    g_signal_connect(G_OBJECT(redo),"clicked",G_CALLBACK(on_redo),editor);

    cut = gtk_tool_button_new_from_stock(GTK_STOCK_CUT);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), cut, -1);
    gtk_tool_item_set_tooltip_text(cut, "Cut the selection");
    g_signal_connect(G_OBJECT(cut),"clicked",G_CALLBACK(on_cut),editor);

    copy = gtk_tool_button_new_from_stock(GTK_STOCK_COPY);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), copy, -1);
    gtk_tool_item_set_tooltip_text(copy, "Copy the selection");
    
    g_signal_connect(G_OBJECT(copy),"clicked",G_CALLBACK(on_copy),editor);

    paste = gtk_tool_button_new_from_stock(GTK_STOCK_PASTE);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), paste, -1);
    gtk_tool_item_set_tooltip_text(paste, "Paste the clipboard's content");
    g_signal_connect(G_OBJECT(paste),"clicked",G_CALLBACK(on_paste),editor);

    /* color = gtk_tool_button_new_from_stock(GTK_STOCK_SELECT_COLOR); */
    /* gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), color, -1); */
    /* gtk_tool_item_set_tooltip_text(color, "Lexical coloration"); */
    /* g_signal_connect(G_OBJECT(color),"clicked",G_CALLBACK(lexical_coloration), editor); */
	
    sep1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), sep1, -1); 


    comp = gtk_tool_button_new_from_stock(GTK_STOCK_EXECUTE);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), comp, -1);
    g_signal_connect(G_OBJECT(comp), "clicked", G_CALLBACK(compile), editor);
    gtk_tool_item_set_tooltip_text(comp, "Compile");

    sep2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), sep2, -1); 

    play = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), play, -1);
    gtk_tool_item_set_tooltip_text(play, "Play");
    g_signal_connect(G_OBJECT(play),"clicked",G_CALLBACK(bmc_play),NULL);

    pause = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PAUSE);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), pause, -1);
    gtk_tool_item_set_tooltip_text(pause, "Pause");
    g_signal_connect(G_OBJECT(pause),"clicked",G_CALLBACK(bmc_pause), NULL);
    
    stop = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), stop, -1);
    gtk_tool_item_set_tooltip_text(stop, "Stop");
    g_signal_connect(G_OBJECT(stop),"clicked",G_CALLBACK(bmc_stop), NULL);
    
    
    sep3 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), sep3, -1); 

    quit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(editor->toolbar), quit, -1);
    g_signal_connect(G_OBJECT(quit), "clicked", G_CALLBACK(window_destroy), editor);
    gtk_tool_item_set_tooltip_text(quit, "Quit");

    	
    // Settings of the icons' size 
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(editor->toolbar),
			      GTK_ICON_SIZE_LARGE_TOOLBAR);
    // Only show icons 
    gtk_toolbar_set_style(GTK_TOOLBAR(editor->toolbar),
			  GTK_TOOLBAR_ICONS);
	
    //Settings of the toolbar's orientation gtk 2. 
    //gtk_toolbar_set_orientation(GTK_TOOLBAR(editor->toolbar),GTK_ORIENTATION_HORIZONTAL);
   

}
