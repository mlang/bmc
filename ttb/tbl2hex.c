/*
 * BRLTTY - A background process providing access to the console screen (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2011 by The BRLTTY Developers.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version. Please see the file LICENSE-GPL for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

#include "prologue.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "file.h"

#include "ttb.h"
#include "ttb_internal.h"

typedef struct {
  void *object;
  const unsigned char *bytes;
  size_t size;
} TableData;

typedef struct {
  const char *extension;
  int (*load) (const char *path, TableData *data);
  void (*unload) (TableData *data);
} TableEntry;

static int
loadTextTable (const char *path, TableData *data) {
  TextTable *table = compileTextTable(path);
  if (!table) return 0;

  data->object = table;
  data->bytes = table->header.bytes;
  data->size = table->size;
  return 1;
}

static void
unloadTextTable (TableData *data) {
  destroyTextTable(data->object);
}

static const TableEntry tableEntries[] = {
  { TEXT_TABLE_EXTENSION, loadTextTable, unloadTextTable }
  ,
  { NULL, NULL, NULL }
};

static const TableEntry *
findTableEntry (const char *extension) {
  const TableEntry *entry = tableEntries;

  while (entry->extension) {
    if (strcmp(entry->extension, extension) == 0) return entry;
    entry += 1;
  }

  logMessage(LOG_ERR, "unrecognized file extension: %s", extension);
  return NULL;
}

int
dumpBytes (FILE *stream, const unsigned char *bytes, size_t count) {
  const unsigned char *byte = bytes;
  const unsigned char *end = byte + count;
  int first = 1;
  int digits;

  if (count) {
    char buffer[0X10];
    snprintf(buffer, sizeof(buffer), "%X", (unsigned int)count-1);
    digits = strlen(buffer);
  } else {
    digits = 1;
  }

  while (byte < end) {
    size_t i;
    for (i = 0; i < 8 && byte < end; ++i) {
      fprintf(stream, " 0X%02X", *byte++);
      if (ferror(stdout)) goto outputError;
      if (byte != end) {
        fprintf(stream, ",");
        if (ferror(stream)) goto outputError;
      }
    }
    fprintf(stream, "\n");
    if (ferror(stdout)) goto outputError;
  } 

  return 1;

outputError:
  logMessage(LOG_ERR, "table write error: %s", strerror(errno));
  return 0;
}

int
main (int argc, char *argv[]) {
  int status;
  char *path;

  --argc, ++argv;

  if (argc == 0) {
    logMessage(LOG_ERR, "missing table file.");
    exit(2);
  }
  path = *argv++, argc--;

  {
    const char *extension = locatePathExtension(path);

    if (extension) {
      const TableEntry *entry = findTableEntry(extension);

      if (entry) {
        TableData data;
        if (entry->load(path, &data)) {
          if (dumpBytes(stdout, data.bytes, data.size)) {
            status = 0;
          } else {
            status = 4;
          }

          entry->unload(&data);
        } else {
          status = 3;
        }
      } else {
        status = 6;
      }
    } else {
      logMessage(LOG_ERR, "no file extension");
      status = 5;
    }
  }

  return status;
}
