/**
 * \file tops.c
 * \brief .ly to .ps and .pdf conversion function. 
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * .ly to .ps and .pdf conversion function. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



/**
 * \fn int tops(char* file_name)
 * \brief This function converts a .ly file to a ps file and a pdf file.
 * \param The file name without .ly
 *
 * This function converts .ly file to a ps file with Lilypond,
 * it edits the links in the ps file in order to match them with
 * the notes' positions in the braille score and finally it converts 
 * the ps file to pdf file.
 */

int tops(char* file_name) {
  char lily_name[1024], ps_name[1024];
  strcpy(lily_name, file_name);
  strcpy(ps_name,   file_name);
  strcat(lily_name, ".ly");
  strcat(ps_name,   ".ps");
  
  char commande[1024];
  strcpy(commande, "lilypond --ps ");
  strcat(commande, lily_name);
  
  system(commande);

  FILE *lily_file, *ps_file;
  lily_file = fopen(lily_name, "r");
  if (lily_file != NULL) {
    ps_file = fopen(ps_name,   "r+");
    if (ps_file != NULL) {
      char sentence[8192];
      char *response, *s_line_column;
      int line, column;
      long ps_fs_pos, ps_sd_pos;
      int i, j;
      char l;

      
      while (fgets(sentence,8192,ps_file) != NULL){
	
	// textedit lines start with a number
	if (isdigit(*sentence)){
	  ps_sd_pos = ftell(ps_file);
	  ps_fs_pos = ftell(ps_file) - strlen(sentence);
	  
	  response = strstr(sentence, "textedit");
	  // if "sentence" is a textedit line
	  if (response != NULL){
	    // get line:column data in .ps
	    ps_fs_pos += ((response - sentence)/ sizeof(char));
	  	    	    
	    response = strstr(response ,":"); 
	    response++;
	    response = strstr(response ,":"); 
	    response++;
	    line = atoi(response);
	    response = strstr(response ,":"); 
	    response++;
	    column = atoi(response);
	    
	    //printf("pos fs-sd : %ld-%ld\n", ps_fs_pos, ps_sd_pos);
	    //printf("line:column (ly) %d:%d\n", line, column);
	    
	    // open the .ly at the position line:column
	    i = line;
	    j = column;
	    
	    
	    fseek(lily_file, 0, SEEK_SET);
	    
	    l = fgetc(lily_file);
	    // go to : line
	    while (l != EOF && i > 1){
	      if (l == '\n') 
		i--;
	      l = fgetc(lily_file);
	    }
	    if (l == EOF)
	      return EXIT_FAILURE;
	    
	    // go to : column
	    while (l != EOF && l != '\n' && j > 0) {
	      j--;
	      l = fgetc(lily_file);
	    }
	    if (l == EOF || l == '\n')
	      return EXIT_FAILURE;
	    
	    // Segmentation fault on key g \major
	    
	    // go to : comment
	    if (fgets(sentence,8192,lily_file) == NULL)
	      return EXIT_FAILURE; // Warging eof ...
	    
	    response = strstr(sentence , "%{");
	    if (response == NULL) {
	      s_line_column = strcpy(sentence,"0:0");
	    } else {
	      response += 2;
	      
	      // printf("%s",response);
	      s_line_column = response;
	      while (*response != '%' && *response != '\0')
		response++;
	      if (*response == '\0')
		strcpy(s_line_column,"0:0"); 
	      *response = '\0';
	      
	    }
	    //printf("s_line_column :%s\n", s_line_column);
	    
	   
	    // write line:column in the .ps
	    
	    fseek(ps_file, ps_fs_pos, SEEK_SET);
	    fputs(s_line_column, ps_file);
	    fputs(") mark_URI " ,ps_file);
	    // fill with space
	    if (ftell(ps_file) > ps_sd_pos)
	      return EXIT_FAILURE;
	    
	    // completion of the line 
	    j = ps_sd_pos - ftell(ps_file) -1; 
	    for (i=0 ; i<j ; i++)
	      fputc(' ', ps_file);
	    //  */	    
	    
	  }
	}
      }
      
      fclose(ps_file);
    } else {
      // oppening error on ps_file
      fprintf(stderr,"Failed opening :%s\n", ps_name);
    }
    fclose(lily_file);
  } else {
    // oppening error on lily_file
    fprintf(stderr,"Failed opening :%s\n", lily_name);
  }

  strcpy(commande, "ps2pdf ");
  strcat(commande, ps_name);
  
  system(commande);
  
  return EXIT_SUCCESS;
}









