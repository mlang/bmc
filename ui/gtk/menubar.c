/**
 * \file menubar.c
 * \brief Menubar creating function.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Menubar creating function.
 *
 */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include "BrailleMusicEditor.h"
#include "new.h"
#include "open.h"
#include "save.h"
#include "edit.h"
#include "window.h"
#include "player.h"
#include "goto.h"
#include "compile.h"
#include "color.h"
/**
 * \def UCHAR_SIZE            
 */

#define NB_RECENT_FILES 5

//array of NB_RECENT_FILES menu items
GtkWidget *recent_files[NB_RECENT_FILES];


/**
 * \fn void open_recent(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief callback when a recent file is activated, it opens the file.
 * \param widget the menu item which triggers the call.
 * \param editor The GUI structure.
 */
void open_recent(GtkWidget *widget, BrailleMusicEditor *editor) {
    const gchar *path = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    open_file_with_path(editor, path);
}

/**
 * \fn static gint sort_mru_func (GtkRecentInfo *a, GtkRecentInfo *b)
 * \brief comparison function between 2 GtkRecentInfo on the date of modification.
 * \param a the first GtkRecentInfo.
 * \param b the second GtkRecentInfo.
 */
static gint sort_mru_func (GtkRecentInfo *a, GtkRecentInfo *b) {
    if (a == b)
	return 0;
    
    if (!a || !b)
	return (a == NULL ? -1 : 1);
    
    return (gtk_recent_info_get_modified (b) - gtk_recent_info_get_modified (a));
}



/**
 * \fn void update_recent_files(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief callback which updating the recent used files.
 * \param widget the widget which triggers the call.
 * \param editor the GUI structure.
 */
void update_recent_files(GtkWidget *widget, BrailleMusicEditor *editor) {
    int i;
    GList *l;
    FILE *cmdfile;
    cmdfile = fopen("/proc/self/cmdline", "r");
    gchar cmdline[1024];
    fgets(cmdline, 1024, cmdfile);
    fclose(cmdfile);
    int cmdline_length = strlen(cmdline);
    int cmd_length = 0;
    for(i=cmdline_length-1; i>=0; i--) {
    	if(cmdline[i] == '/')
    	    break;
    	cmd_length++;
    }
    gchar cmd[cmd_length+1];
    for(i=0; i<=cmd_length; i++)
    	cmd[i] = cmdline[cmdline_length-cmd_length+i];
    
    GtkWidget *recent_chooser = 
	gtk_recent_chooser_widget_new_for_manager(gtk_recent_manager_get_default());
    GtkRecentFilter *filter = gtk_recent_filter_new ();
    gtk_recent_filter_set_name (filter, "recent file");
    gtk_recent_filter_add_application (filter, cmd);
    gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(recent_chooser), filter);
    
    GList *items = gtk_recent_chooser_get_items(GTK_RECENT_CHOOSER(recent_chooser));
    
    items = g_list_sort(items, (GCompareFunc)sort_mru_func);

    for(i = 0, l = items; i < NB_RECENT_FILES && l != NULL; i +=1, l = l->next) {
    	GtkRecentInfo *info = l->data;
    	   	
	const gchar *uri = gtk_recent_info_get_uri(info);
	GFile *file = g_file_new_for_uri(uri);
	
	gchar *path = g_file_get_path(file);
	gtk_menu_item_set_label(GTK_MENU_ITEM(recent_files[i]), path);
	gtk_widget_set_sensitive(recent_files[i], TRUE);
	
	g_signal_connect(G_OBJECT(recent_files[i]), "activate", 
			 G_CALLBACK(open_recent), editor);
	g_free(path);
	g_object_unref(file);
	
    }
    g_list_foreach (items, (GFunc) gtk_recent_info_unref, NULL);
    g_list_free (items);
}




/**
 * \fn void create_menubar(BrailleMusicEditor *editor)
 * \brief This function creates the menubar of the window 
 * (the menu containing open, edit...).
  * \param editor The GUI structure.
 */
void create_menubar(BrailleMusicEditor *editor)
{
    editor->menubar = gtk_menu_bar_new();
    GtkAccelGroup *accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(editor->window), accel_group);
	
    //creation of the file submenu
    GtkWidget *filemenu = gtk_menu_new();
    
    //creation of the file submenu's items
    GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
    g_signal_connect(G_OBJECT(file), "activate", 
		     G_CALLBACK(update_recent_files), editor);
    
    GtkWidget *new = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, 
							accel_group);
    g_signal_connect(G_OBJECT(new), "activate", G_CALLBACK(new_file), editor);
    gtk_widget_add_accelerator(new, "activate", accel_group,GDK_KEY_n, 
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    GtkWidget *open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, 
							 accel_group);
    g_signal_connect(G_OBJECT(open), "activate", G_CALLBACK(open_file), editor);
    gtk_widget_add_accelerator(open, "activate", accel_group,GDK_KEY_o, 
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    GtkWidget *sep1 = gtk_separator_menu_item_new();
    GtkWidget *save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, 
							 accel_group);
    g_signal_connect(G_OBJECT(save), "activate", G_CALLBACK(save_file), editor);
    gtk_widget_add_accelerator(save, "activate", accel_group,GDK_KEY_s,
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);	
    GtkWidget *saveas = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, 
							   accel_group);
    g_signal_connect(G_OBJECT(saveas), "activate", 
		     G_CALLBACK(save_file_as), editor);
    gtk_widget_add_accelerator(saveas, "activate", accel_group,GDK_KEY_s, 
			       GDK_CONTROL_MASK | GDK_SHIFT_MASK, 
			       GTK_ACCEL_VISIBLE);
    GtkWidget *sep2= gtk_separator_menu_item_new();
    
    int i;
    for(i=0; i<NB_RECENT_FILES; i++) {
	recent_files[i] = gtk_menu_item_new_with_label("Recent Used File");
	gtk_widget_set_sensitive(recent_files[i],FALSE);
    }

    GtkWidget *sep3= gtk_separator_menu_item_new();
    GtkWidget *quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    g_signal_connect(G_OBJECT(quit), "activate", 
		     G_CALLBACK(window_destroy), editor);
    gtk_widget_add_accelerator(quit, "activate", accel_group,GDK_KEY_q, 
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 

    //addition of the file submenu's items in the menu 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep1);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), save);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), saveas);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep2);
    for(i=0; i<NB_RECENT_FILES; i++)
    	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), recent_files[i]);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep3);
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
    gtk_menu_shell_append(GTK_MENU_SHELL(editor->menubar), file);


    //creation of the edit submenu	
    GtkWidget *editmenu = gtk_menu_new();
    
    //creation of the edit submenu's items
    GtkWidget *edit = gtk_menu_item_new_with_mnemonic("_Edit");
	
    GtkWidget *undo = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO,
							 accel_group);
    g_signal_connect(G_OBJECT(undo), "activate", G_CALLBACK(on_undo), editor);
    gtk_widget_add_accelerator(undo, "activate", accel_group,GDK_KEY_z, 
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    GtkWidget *redo = gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO, 
							 accel_group);
    g_signal_connect(G_OBJECT(redo), "activate", G_CALLBACK(on_redo), editor);
    gtk_widget_add_accelerator(redo, "activate", accel_group,GDK_KEY_z, 
			       GDK_CONTROL_MASK | GDK_SHIFT_MASK, 
			       GTK_ACCEL_VISIBLE);
    GtkWidget *sep4= gtk_separator_menu_item_new();
    GtkWidget *cut = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, 
							accel_group);
    g_signal_connect(G_OBJECT(cut), "activate", G_CALLBACK(on_cut), editor);
    GtkWidget *copy = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, 
							 accel_group);
    g_signal_connect(G_OBJECT(copy), "activate", G_CALLBACK(on_copy), editor);
    GtkWidget *paste = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, 
							  accel_group);
    g_signal_connect(G_OBJECT(paste), "activate", G_CALLBACK(on_paste), editor);
    GtkWidget *sep5= gtk_separator_menu_item_new();
    GtkWidget *next= gtk_image_menu_item_new_from_stock(GTK_STOCK_GO_FORWARD, 
							accel_group);
    g_signal_connect(G_OBJECT(next), "activate", G_CALLBACK(goto_next), editor);
    gtk_widget_add_accelerator(next, "activate", accel_group,GDK_KEY_Right, 
			       GDK_CONTROL_MASK | GDK_SHIFT_MASK 
			       , GTK_ACCEL_VISIBLE);	
    GtkWidget *prev = gtk_image_menu_item_new_from_stock(GTK_STOCK_GO_BACK,
							 accel_group);
    g_signal_connect(G_OBJECT(prev), "activate", G_CALLBACK(goto_prev), editor);
    gtk_widget_add_accelerator(prev, "activate", accel_group,GDK_KEY_Left,
			       GDK_CONTROL_MASK | GDK_SHIFT_MASK , 
			       GTK_ACCEL_VISIBLE);	
    GtkWidget *goto_n = gtk_image_menu_item_new_from_stock(GTK_STOCK_JUMP_TO, 
							   accel_group);
    g_signal_connect(G_OBJECT(goto_n), "activate", G_CALLBACK(goto_num),editor);
    gtk_widget_add_accelerator(goto_n, "activate", accel_group,GDK_KEY_g,
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);	
    GtkWidget *sep6= gtk_separator_menu_item_new();
    GtkWidget *select_all = gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_ALL,
							       accel_group);
    g_signal_connect(G_OBJECT(select_all), "activate", G_CALLBACK(on_select_all),
		     editor);
    gtk_widget_add_accelerator(select_all, "activate", accel_group,GDK_KEY_a,
			       GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    
    //addition of the edit submenu's items in the menu 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),undo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),redo);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),sep4);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),cut);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),copy);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),paste);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),sep5);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),next);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),prev);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),goto_n);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),sep6);
    //	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),select);
    gtk_menu_shell_append(GTK_MENU_SHELL(editmenu),select_all);
    gtk_menu_shell_append(GTK_MENU_SHELL(editor->menubar), edit);

    //creation of the build submenu	
    GtkWidget *buildmenu = gtk_menu_new();
    
    //creation of the build submenu's items
    GtkWidget *build = gtk_menu_item_new_with_mnemonic("_Build");
    GtkWidget *comp = gtk_menu_item_new_with_label("Compile");
    gtk_widget_add_accelerator(comp, "activate", accel_group,GDK_KEY_F4,
			       (GdkModifierType)0, GTK_ACCEL_VISIBLE);	
    g_signal_connect(G_OBJECT(comp), "activate", G_CALLBACK(compile), editor);
    
    //addition of the buid submenu's items in the menu 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(build), buildmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(buildmenu), comp);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(editor->menubar), build);


    //creation of the player submenu
    GtkWidget *playermenu = gtk_menu_new();
    
    //creation of the player submenu's items
    GtkWidget *player = gtk_menu_item_new_with_mnemonic("_Player");
    GtkWidget *play = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PLAY,
							 accel_group);
    g_signal_connect(G_OBJECT(play), "activate", G_CALLBACK(bmc_play), NULL);
    gtk_widget_add_accelerator(play, "activate", accel_group,GDK_KEY_F5,
			       (GdkModifierType)0, GTK_ACCEL_VISIBLE);	
    
    GtkWidget *pause = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, 
							  accel_group);
    g_signal_connect(G_OBJECT(pause), "activate", G_CALLBACK(bmc_pause), NULL);
    gtk_widget_add_accelerator(pause, "activate", accel_group,GDK_KEY_F6,
			       (GdkModifierType)0, GTK_ACCEL_VISIBLE);	
    
    GtkWidget *stop = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_STOP,
							 accel_group);
    g_signal_connect(G_OBJECT(stop), "activate", G_CALLBACK(bmc_stop), NULL);
    gtk_widget_add_accelerator(stop, "activate", accel_group,GDK_KEY_F7, 
			       (GdkModifierType)0, GTK_ACCEL_VISIBLE);	
    
    //addition of the player submenu's items in the menu 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(player), playermenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(playermenu), play);
    gtk_menu_shell_append(GTK_MENU_SHELL(playermenu), pause);
    gtk_menu_shell_append(GTK_MENU_SHELL(playermenu), stop);

    gtk_menu_shell_append(GTK_MENU_SHELL(editor->menubar), player);

    //creation of the options submenu
    GtkWidget *optionsmenu = gtk_menu_new();
    
    //creation of the options submenu's items
    GtkWidget *options = gtk_menu_item_new_with_mnemonic("_Options");
    GtkWidget *enable_color = gtk_check_menu_item_new_with_label("Enable Lexical Coloration");
    GtkWidget *color = gtk_menu_item_new_with_label("Color Options");
    gtk_widget_add_accelerator(enable_color, "activate", accel_group,GDK_KEY_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);	
    g_signal_connect(G_OBJECT(color), "activate", 
		     G_CALLBACK(color_options), editor);
    g_signal_connect(G_OBJECT(enable_color), "activate", 
		     G_CALLBACK(lexical_coloration), editor);
    

    //addition of the options submenu's items in the menu 
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(options), optionsmenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsmenu), enable_color);
    gtk_menu_shell_append(GTK_MENU_SHELL(optionsmenu), color);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(editor->menubar), options);  
}
