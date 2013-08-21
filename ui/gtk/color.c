/**
 * \file color.c
 * \brief Braille Music lexical coloration.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Lexical Highlighting callback functions
 *
 */


#include <gtk/gtk.h> 
#include "BrailleMusicEditor.h"
#include "color.h"
#include <string.h>

/**
 * \def NB_TYPES              
 * \def NB_ACCIDENTALS         
 * \def NB_ARTICULATIONS      
 * \def NB_CLEFS              
 * \def NB_FINGERINGS         
 * \def NB_IN_ACCORDS         
 * \def NB_INTERVALS           
 * \def NB_NOTES              
 * \def NB_OCTAVES            
 * \def NB_ORNAMENTS          
 * \def NB_REPETITONS_TREMOLO 
 * \def NB_RESTS              
 * \def NB_SLURS_TIES         
 * \def NB_STEMS              
 * \def NB_TUPLETS            
 * \def UCHAR_SIZE            
 */
  
 
#define NB_TYPES              17
#define NB_ACCIDENTALS         3
#define NB_ARTICULATIONS      13
#define NB_CLEFS               3
#define NB_FINGERINGS          5
#define NB_IN_ACCORDS          3
#define NB_INTERVALS           7
#define NB_NOTES              28
#define NB_OCTAVES             7
#define NB_ORNAMENTS           9
#define NB_REPETITONS_TREMOLO 10
#define NB_RESTS               4
#define NB_SLURS_TIES         13
#define NB_STEMS               6
#define NB_TUPLETS             6
#define UCHAR_SIZE             3


static GdkColor color_table[NB_TYPES];

static char type_table[NB_TYPES][22] = {
    "Accidental",
    "Articulation",
    "Bar",
    "Clef",
    "Dot",
    "Fingering",
    "In accord",
    "Intervals",
    "Note",
    "Octave",
    "Ornament",
    "Repetitons & Tremolo",
    "Rest",
    "Slur & Tie",
    "Stem",
    "Tuplet",
    "Right or Left hand"
};

static char default_color_table[NB_TYPES][8] = {
    "#FF0000",
    "#00FFDB",
    "#FF4C00",
    "#C7DE3E",
    "#719AB9",
    "#C38B02",
    "#5E5402",
    "#FFE827",
    "#0000FF",
    "#EB6397",
    "#00FF00",
    "#5D0000",
    "#72008B",
    "#A4A1A5",
    "#4DCD99",
    "#C02492",
    "#60754E",
};


static GHashTable *braille_table; 

static gchar accidentals[NB_ACCIDENTALS][UCHAR_SIZE+1] = {
    "⠩", //sharp
    "⠣", //flat
    "⠡"  //natural 
};

static gchar articulations[NB_ARTICULATIONS][UCHAR_SIZE*2+1] = {
    "⠦",  // a dot
    "⠠⠦", //a pear-shaped 
    "⠐⠦", //a dot \a line 
    "⠸⠦", //a line 
    "⠨⠦", // >
    "⠈⠦", // <
    "⠰⠦", // ^
    "⠣⠇", // Fermata
    "⠜⠂", // a comma
    "⠜⠅", // A vertical wavy line or curve 
    "⠡⠄", // <>
    "⠜⠉", // Beginning of diverging lines
    "⠜⠒"  // End of diverging lines   
};

//static gchar measure_bar[] = "⠀";
static gchar ending_double_bar[] = "⠣⠅";
static gchar sectional_double_bar[] = "⠣⠅⠄";

static gchar clefs[NB_CLEFS][UCHAR_SIZE*3+1] = {
    "⠜⠌⠇", //G
    "⠜⠼⠇", //F
    "⠜⠬⠇"  //C
};

static gchar dot[] = "⠄";

static gchar fingerings[NB_FINGERINGS][UCHAR_SIZE+1] = {
    "⠁", //1
    "⠃", //2
    "⠇", //3
    "⠂", //4
    "⠅"  //5
};

static gchar in_accords[NB_IN_ACCORDS][UCHAR_SIZE*2+1] = {
    "⠣⠜", 
    "⠐⠂",
    "⠨⠅"
};

static gchar intervals[NB_INTERVALS][UCHAR_SIZE+1] = {
    "⠌", //2nd
    "⠬", //3th
    "⠼", //4th
    "⠔", //5th
    "⠴", //6th
    "⠒", //7th
    "⠤"  //8th
};

static gchar notes[NB_NOTES][UCHAR_SIZE+1] = {
    "⠽", //C
    "⠵", //D
    "⠯", //E
    "⠿", //F
    "⠷", //G
    "⠮", //A
    "⠾", //B

    "⠝", //C2(half)
    "⠕", //D2
    "⠏", //E2
    "⠟", //F2
    "⠗", //G2
    "⠎", //A2
    "⠞", //B2

    "⠹", //C4(quarter)
    "⠱", //D4
    "⠫", //E4
    "⠻", //F4
    "⠳", //G4
    "⠪", //A4
    "⠺", //B4

    "⠙", //C8(eighth)
    "⠑", //D8
    "⠋", //E8
    "⠛", //F8
    "⠓", //G8
    "⠊", //A8
    "⠚", //B8
};

static gchar octaves[NB_OCTAVES][UCHAR_SIZE+1] = {
    "⠈", //1st
    "⠘", //2nd
    "⠸", //3th
    "⠐", //4th
    "⠨", //5th
    "⠰", //6th
    "⠠"  //7th
};

static gchar ornaments[NB_ORNAMENTS][UCHAR_SIZE*2+1] = {
    "⠢",  //Short appoggiatura 
    "⠐⠢", //Long appoggiatura 
    "⠖",  //Trill
    "⠲",  //Turn
    "⠠⠲", //Turn
    "⠲⠇", //Turn
    "⠐⠖", // mordent
    "⠰⠖", //Extented mordent
    "⠖⠇" 
};

static gchar repetitons_tremolo[NB_REPETITONS_TREMOLO][UCHAR_SIZE*2+1] = {
    "⠘⠃", //8th
    "⠘⠇", //16th
    "⠘⠂", //32th
    "⠘⠅", //64th
    "⠘⠄", //128th
    "⠨⠃", //8th
    "⠨⠇", //16th
    "⠨⠂", //32th
    "⠨⠁", //64th
    "⠨⠄"  //128th
};

static gchar rests[NB_RESTS][UCHAR_SIZE+1] = {
    "⠍", 
    "⠥", 
    "⠧",
    "⠭"
};

static gchar slurs_ties[NB_SLURS_TIES][UCHAR_SIZE*2+1] = {
    "⠉",
    "⠰⠃",
    "⠘⠆",
    "⠠⠉",
    "⠸⠉",
    "⠐⠉",
    "⠈⠇",
    "⠐⠇",
    "⠰⠉",
    "⠈⠆",
    "⠈⠉",
    "⠨⠉",
    "⠘⠉"
};

static gchar stems[NB_STEMS][UCHAR_SIZE*2+1] = {
    "⠸⠄", //whole
    "⠸⠅", //half
    "⠸⠁", //quater
    "⠸⠃", //8th
    "⠸⠇", //16th
    "⠸⠂"  //32th
};

static gchar tuplets[NB_TUPLETS][UCHAR_SIZE*2+1] = {
    "⠰⠂", 
    "⠐⠆", 
    "⠰⠄", 
    "⠠⠆", 
    "⠐⠄", 
    "⠠⠂" 
};


static gchar right_hand[] = "⠨⠜";
static gchar left_hand[] = "⠸⠜";

static int coloration_enabled = 0;

/**
 * \fn void lexical_coloration(GtkButton *widget, BrailleMusicEditor *editor)
 * \brief This function enables/disables the lexical highlighting.
 * \param widget The button trigerring the function.
 * \param editor The GUI structure. 
 */
void lexical_coloration(GtkWidget *widget, BrailleMusicEditor *editor) {	
    init_colors();
    
    if(coloration_enabled%2 == 0)
	color(editor);
    else
        lexical_coloration_off(editor);
	coloration_enabled ++;
}


/**
 * \fn void coloration_update(GtkWidget *widget, BrailleMusicEditor *editor)
 * \brief This function updates the lexical highlighting if it's enabled.
 * \param widget The button trigerring the function.
 * \param editor The GUI structure. 
 */
void coloration_update(GtkWidget *widget, BrailleMusicEditor *editor) {
    if(coloration_enabled%2 == 1)
	color(editor);    
}


/**
 * \fn void lexical_coloration_off(BrailleMusicEditor *editor)
 * \brief This function removes the lexical highlighting.
 * \param editor The GUI structure.
 */
void lexical_coloration_off(BrailleMusicEditor *editor) {
    GtkTextIter start, end;
    GtkTextBuffer *buffer;
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)); 
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
}
/**
 * \fn void init_braille_table()
 * \brief This function associates each braille pattern with its type
 */
void init_braille_table() {
    braille_table = g_hash_table_new(g_str_hash, g_str_equal);
    int i;
    for(i=0; i<NB_ACCIDENTALS; i++) 
	g_hash_table_insert(braille_table, accidentals[i], type_table[0]);
    
    for(i=0; i<NB_ARTICULATIONS; i++) 
	g_hash_table_insert(braille_table, articulations[i], type_table[1]);

    //g_hash_table_insert(braille_table, measure_bar, type_table[1]);
    g_hash_table_insert(braille_table, ending_double_bar, type_table[2]);
    g_hash_table_insert(braille_table, sectional_double_bar, type_table[2]);
    
    for(i=0; i<NB_CLEFS; i++) 
	g_hash_table_insert(braille_table, clefs[i], type_table[3]);
    
    g_hash_table_insert(braille_table, dot, type_table[4]);

    for(i=0; i<NB_FINGERINGS; i++) 
	g_hash_table_insert(braille_table, fingerings[i], type_table[5]);

    for(i=0; i<NB_IN_ACCORDS; i++) 
	g_hash_table_insert(braille_table, in_accords[i], type_table[6]);
    
    for(i=0; i<NB_INTERVALS; i++) 
	g_hash_table_insert(braille_table, intervals[i], type_table[7]);

    for(i=0; i<NB_NOTES; i++) 
	g_hash_table_insert(braille_table, notes[i], type_table[8]);

    for(i=0; i<NB_OCTAVES; i++) 
	g_hash_table_insert(braille_table, octaves[i], type_table[9]);
    
    for(i=0; i<NB_ORNAMENTS; i++) 
	g_hash_table_insert(braille_table, ornaments[i], type_table[10]);
    
    for(i=0; i<NB_REPETITONS_TREMOLO; i++) 
	  g_hash_table_insert(braille_table, repetitons_tremolo[i], type_table[11]);
      
    for(i=0; i<NB_RESTS; i++) 
	g_hash_table_insert(braille_table, rests[i], type_table[12]);
    
    for(i=0; i<NB_SLURS_TIES; i++) 
	g_hash_table_insert(braille_table, slurs_ties[i], type_table[13]);

    for(i=0; i<NB_STEMS; i++) 
	g_hash_table_insert(braille_table, stems[i], type_table[14]);

    for(i=0; i<NB_TUPLETS; i++) 
	g_hash_table_insert(braille_table, tuplets[i], type_table[15]);
	
    g_hash_table_insert(braille_table, right_hand, type_table[16]);
    g_hash_table_insert(braille_table, left_hand, type_table[16]);
}

/**
 * \fn void init_colors()
 * \brief This function initiates the GdkColor table
 */
void init_colors() {
    static int initialised = 0;
    if(!initialised) {
	int i;
	for(i = 0; i < NB_TYPES; i++) 
	    gdk_color_parse(default_color_table[i], &color_table[i]);
	
	initialised = 1;
    }
}

/**
 * \fn void set_tags(GtkTextBuffer *buffer)
 * \brief This function creates a tag for each color 
 * which is in the GdkColor table
 */
void set_tags(GtkTextBuffer *buffer) {
    GtkTextIter start, end;
    GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(buffer);

    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    
    GtkTextTag *tag;
    int i;
    for(i = 0; i< NB_TYPES; i++) {
	tag = gtk_text_tag_table_lookup(tag_table, type_table[i]);
	if(tag) 
	    gtk_text_tag_table_remove(tag_table, tag);
	
	gtk_text_buffer_create_tag(buffer, type_table[i], "foreground", 
				   gdk_color_to_string(&color_table[i]), NULL); 
    }
}

/**
 * \fn void color(BrailleMusicEditor *editor)
 * \brief This function color the differents types of braille music notations
 * in the textview.
 * \param editor The GUI structure.
 * 
 * This function will color the diffrents types of braille music notations present in text. 
 */
void color(BrailleMusicEditor *editor)
{
    GtkTextIter start, end;
    GtkTextIter start_match, end_match, start_match2, start_match3;

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)); 
    
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    
    gtk_text_buffer_get_start_iter(buffer, &start_match);
    gtk_text_buffer_get_start_iter(buffer, &end_match);
    
    set_tags(buffer);
    init_braille_table();
    
    do {
	gtk_text_iter_forward_chars(&end_match, 1);
	gchar *c = gtk_text_iter_get_slice(&start_match, &end_match);
	
	start_match2 = start_match;
	gtk_text_iter_backward_chars(&start_match2, 1);
	gchar *c2 = gtk_text_iter_get_slice(&start_match2, &end_match);
	
	start_match3 = start_match2;
	gtk_text_iter_backward_chars(&start_match3, 1);
	gchar *c3 = gtk_text_iter_get_slice(&start_match3, &end_match);

	gchar *type;
	if((type = g_hash_table_lookup(braille_table, c3)) != NULL) {
	    gtk_text_buffer_remove_all_tags(buffer, &start_match3, &end_match);
	    gtk_text_buffer_apply_tag_by_name(buffer, type, &start_match3, &end_match);
	}
	else if((type = g_hash_table_lookup(braille_table, c2)) != NULL) {
	    gtk_text_buffer_remove_all_tags(buffer, &start_match2, &end_match);
	    gtk_text_buffer_apply_tag_by_name(buffer, type, &start_match2, &end_match);
	}
	else if((type = g_hash_table_lookup(braille_table, c)) != NULL) {
	    gtk_text_buffer_apply_tag_by_name(buffer, type, &start_match, &end_match);
	}
	
    } while(gtk_text_iter_forward_chars(&start_match, 1));
}



/**
 * \fn void color_options(GtkWidget *widget, BrailleMusicEditor *editor) 
 * \brief Color options callback
 * \param editor The GUI structure .
 * 
 * This function shows a dialog which allows user to customize the colors
 * used in the lexical highlighting. 
 */
void color_options(GtkWidget *widget, BrailleMusicEditor *editor) {
    GtkWidget* dialog;
    
    // Creation of a dialog window
    dialog = gtk_dialog_new_with_buttons("Lexical Highlighting",
 					 GTK_WINDOW(editor->window),
					 GTK_DIALOG_MODAL,
					 GTK_STOCK_CANCEL,
					 GTK_RESPONSE_CANCEL,
					 GTK_STOCK_OK,
					 GTK_RESPONSE_OK,
					 NULL);  

    gtk_window_set_default_size(GTK_WINDOW(dialog), 220, -1);
    
    init_colors();
    
    GtkWidget *buttons[NB_TYPES];
    GtkWidget *labels[NB_TYPES];
    GtkTextBuffer *buffer;
    int i;
    GtkWidget *box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    for(i = 0; i < NB_TYPES; i++) {
       	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
	labels[i] = gtk_label_new(type_table[i]);
	//buttons[i] = gtk_button_new_with_label(type_table[i]);
	buttons[i] = gtk_color_button_new_with_color(&color_table[i]);
	gtk_box_pack_start(GTK_BOX(hbox), labels[i], TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), buttons[i], TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, 0);
	
    	gtk_widget_modify_fg(labels[i], GTK_STATE_NORMAL, &color_table[i]);
    }
    gtk_widget_show_all(box);
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->textview)); 
   
    // run the dialog window
    switch (gtk_dialog_run(GTK_DIALOG(dialog)))
	{
	case GTK_RESPONSE_OK:
	    for(i = 0; i < NB_TYPES; i++) {
		gtk_color_button_get_color(GTK_COLOR_BUTTON(buttons[i]), 
					   &color_table[i]); 
	    }
	    //updating the coloration
	    set_tags(buffer);
	    coloration_update(widget, editor);
	    break;
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_NONE:
	default:
	    break;
	}
 
    // Destruction of the dialog window
    gtk_widget_destroy((GtkWidget *) dialog); 
 
}
