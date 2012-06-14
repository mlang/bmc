#ifndef ERRORS_H
#define ERRORS_H

/**
 * \file errors.h
 * \brief Error alert functions.
 * \author Team BMC
 * \version 1.0
 * \date 07 April 2012
 *
 * Error alert function prototypes.
 *
 */


void open_error(GtkWidget *file_selection, FILE *file, 
		const gchar * message_format, const gchar* path);

#endif
