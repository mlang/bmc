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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __MINGW32__
/* MinGW defines localtime_r() in <pthread.h> */
#include <pthread.h>
#endif /* __MINGW32__ */

#ifdef __MSDOS__
/* DJGCC doesn't define localtime_r() but localtime() is safe */
static inline struct tm *
localtime_r (const time_t *timep, struct tm *result) {
  *result = *localtime(timep);
  return result;
}
#endif /* __MSDOS__ */

#include "log.h"

static int printLevel = LOG_NOTICE;
static int logLevel = LOG_NOTICE;
static const char *logPrefix = NULL;

int
setLogLevel (int newLevel) {
  int oldLevel = logLevel;
  logLevel = newLevel;
  return oldLevel;
}

const char *
setLogPrefix (const char *newPrefix) {
  const char *oldPrefix = logPrefix;
  logPrefix = newPrefix;
  return oldPrefix;
}

int
setPrintLevel (int newLevel) {
  int oldLevel = printLevel;
  printLevel = newLevel;
  return oldLevel;
}

int
setPrintOff (void) {
  return setPrintLevel(-1);
}

void
logData (int level, LogDataFormatter *formatLogData, const void *data) {
  if (level <= printLevel) {
    int reason = errno;
    char buffer[0X1000];
    const char *record = formatLogData(buffer, sizeof(buffer), data);

    if (*record) {
      FILE *stream = stderr;

      if (logPrefix) {
        fputs(logPrefix, stream);
        fputs(": ", stream);
      }

      fputs(record, stream);
      fputc('\n', stream);
      fflush(stream);
    }

    errno = reason;
  }
}

typedef struct {
  const char *format;
  va_list *arguments;
} LogMessageData;

static const char *
formatLogMessageData (char *buffer, size_t size, const void *data) {
  const LogMessageData *msg = data;
  vsnprintf(buffer, size, msg->format, *msg->arguments);
  return buffer;
}

void
logMessage (int level, const char *format, ...) {
  va_list arguments;
  const LogMessageData msg = {
    .format = format,
    .arguments = &arguments
  };

  va_start(arguments, format);
  logData(level, formatLogMessageData, &msg);
  va_end(arguments);
}

void
logSystemError (const char *action) {
  logMessage(LOG_ERR, "%s error %d: %s.", action, errno, strerror(errno));
}
#ifdef WINDOWS
void
logWindowsError (DWORD error, const char *action) {
  char *message;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (char *)&message, 0, NULL);

  {
    char *end = strpbrk(message, "\r\n");
    if (end) *end = 0;
  }

  logMessage(LOG_ERR, "%s error %ld: %s", action, error, message);
  LocalFree(message);
}

void
logWindowsSystemError (const char *action) {
  DWORD error = GetLastError();
  logWindowsError(error, action);
}

#endif /* WINDOWS */
