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
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>

#if defined(WINDOWS)

#elif defined(__MSDOS__)
#include "sys_msdos.h"

#else /* Unix */
#include <langinfo.h>
#endif /* platform-specific includes */

#include "log.h"
#include "misc.h"
#include "charset.h"
#include "unicode.h"

static char *currentCharset = NULL;

#if defined(HAVE_ICONV_H)
#include <iconv.h>

#define CHARSET_ICONV_NULL ((iconv_t)-1)
#define CHARSET_ICONV_HANDLE(name) iconv_t iconv##name = CHARSET_ICONV_NULL

static CHARSET_ICONV_HANDLE(CharToWchar);
static CHARSET_ICONV_HANDLE(WcharToChar);

#define CHARSET_CONVERT_TYPE_TO_TYPE(name, from, to, ret, eof) \
ret convert##name (from f) { \
  if (getCharset()) { \
    from *fp = &f; \
    size_t fs = sizeof(f); \
    to t; \
    to *tp = &t; \
    size_t ts = sizeof(t); \
    if (iconv(iconv##name, (void *)&fp, &fs, (void *)&tp, &ts) != (size_t)-1) return t; \
    logMessage(LOG_DEBUG, "iconv (" #from " -> " #to ") error: %s", strerror(errno)); \
  } \
  return eof; \
}
CHARSET_CONVERT_TYPE_TO_TYPE(CharToWchar, char, wchar_t, wint_t, WEOF)
CHARSET_CONVERT_TYPE_TO_TYPE(WcharToChar, wchar_t, unsigned char, int, EOF)
#undef CHARSET_CONVERT_TYPE_TO_TYPE

#elif defined(__MINGW32__)

#if defined(CP_THREAD_ACP)
#define CHARSET_WINDOWS_CODEPAGE CP_THREAD_ACP
#else /* Windows codepage */
#define CHARSET_WINDOWS_CODEPAGE CP_ACP
#endif /* Windows codepage */

wint_t
convertCharToWchar (char c) {
  wchar_t wc;
  int result = MultiByteToWideChar(CHARSET_WINDOWS_CODEPAGE, MB_ERR_INVALID_CHARS,
                                   &c, 1,  &wc, 1);
  if (result) return wc;
  logWindowsSystemError("MultiByteToWideChar[" STRINGIFY(CHARSET_WINDOWS_CODEPAGE) "]");
  return WEOF;
}

int
convertWcharToChar (wchar_t wc) {
  unsigned char c;
  int result = WideCharToMultiByte(CHARSET_WINDOWS_CODEPAGE, WC_NO_BEST_FIT_CHARS /* WC_ERR_INVALID_CHARS */,
                                   &wc, 1, &c, 1,
                                   NULL, NULL);
  if (result) return c;
  logWindowsSystemError("WideCharToMultiByte[" STRINGIFY(CHARSET_WINDOWS_CODEPAGE) "]");
  return EOF;
}

#else /* conversions */
#error "Character conversion"
#endif /* conversions */

wint_t
convertUtf8ToWchar (const char **utf8, size_t *utfs) {
  wint_t wc = WEOF;
  int state = 0;

  while (*utfs) {
    unsigned char byte = *(*utf8)++;
    (*utfs)--;

    if (!(byte & 0X80)) {
      if (wc != WEOF) goto truncated;
      wc = byte;
      break;
    }

    if (!(byte & 0X40)) {
      if (wc == WEOF) break;
      wc = (wc << 6) | (byte & 0X3F);
      if (!--state) break;
    } else {
      if (!(byte & 0X20)) {
        state = 1;
      } else if (!(byte & 0X10)) {
        state = 2;
      } else if (!(byte & 0X08)) {
        state = 3;
      } else if (!(byte & 0X04)) {
        state = 4;
      } else if (!(byte & 0X02)) {
        state = 5;
      } else {
        state = 0;
      }

      if (wc != WEOF) goto truncated;

      if (!state) {
        wc = WEOF;
        break;
      }

      wc = byte & ((1 << (6 - state)) - 1);
    }
  }

  while (*utfs) {
    if ((**utf8 & 0XC0) != 0X80) break;
    (*utf8)++, (*utfs)--;
    wc = WEOF;
  }

  return wc;

truncated:
  (*utf8)--, (*utfs)++;
  return WEOF;
}

void
convertStringToWchars (const char **utf8, wchar_t **characters, size_t count) {
  while (**utf8 && (count > 1)) {
    size_t utfs = UTF8_LEN_MAX;
    wint_t character = convertUtf8ToWchar(utf8, &utfs);

    if (character == WEOF) break;
    *(*characters)++ = character;
    count -= 1;
  }

  if (count) **characters = 0;
}

static const char *
getLocaleCharset (void) {
#if defined(__MSDOS__)
  static char codepage[8];
  snprintf(codepage, sizeof(codepage), "CP%03u", getCodePage());
  return codepage;
#else /* use setlocale() */
  const char *locale = setlocale(LC_ALL, "");

  if (locale &&
      (strcmp(locale, "C") != 0) &&
      (strcmp(locale, "POSIX") != 0)) {
    /* some 8-bit locale is set, assume its charset is correct */
#if defined(WINDOWS)
    static char codepage[8] = {'C', 'P'};
    GetLocaleInfo(GetThreadLocale(), LOCALE_IDEFAULTANSICODEPAGE, codepage+2, sizeof(codepage)-2);
    return codepage;
#else /* Unix */
    return nl_langinfo(CODESET);
#endif /* locale character set */
  }
  return "ISO-8859-1";
#endif /* character set */
}

const char *
getWcharCharset (void) {
  static const char *wcharCharset = NULL;

  if (!wcharCharset) {
    char charset[0X10];
    snprintf(charset, sizeof(charset), "UCS-%lu%cE",
             (unsigned long)sizeof(wchar_t),
#ifdef WORDS_BIGENDIAN
             'B'
#else /* WORDS_BIGENDIAN */
             'L'
#endif /* WORDS_BIGENDIAN */
            );

    wcharCharset = strdup(charset);
  }

  return wcharCharset;
}

static const char *
setCharset (const char *name) {
  char *charset;

  if (name) {
    if (currentCharset && (strcmp(currentCharset, name) == 0)) return currentCharset;
  } else if (currentCharset) {
    return currentCharset;
  } else {
    name = getLocaleCharset();
  }
  if (!(charset = strdup(name))) return NULL;

#if defined(HAVE_ICONV_H)
  {
    const char *const wcharCharset = getWcharCharset();

    typedef struct {
      iconv_t *handle;
      const char *fromCharset;
      const char *toCharset;
      unsigned permanent:1;
      iconv_t newHandle;
    } ConvEntry;

    ConvEntry convTable[] = {
      {&iconvCharToWchar, charset, wcharCharset, 0, CHARSET_ICONV_NULL},
      {&iconvWcharToChar, wcharCharset, charset, 0, CHARSET_ICONV_NULL},
      {NULL, NULL, NULL, 1, CHARSET_ICONV_NULL}
    };
    ConvEntry *conv = convTable;

    while (conv->handle) {
      if (conv->permanent && (*conv->handle != CHARSET_ICONV_NULL)) {
        conv->newHandle = CHARSET_ICONV_NULL;
      } else if ((conv->newHandle = iconv_open(conv->toCharset, conv->fromCharset)) == CHARSET_ICONV_NULL) {
        logSystemError("iconv_open");
        while (conv != convTable)
          if ((--conv)->newHandle != CHARSET_ICONV_NULL)
            iconv_close(conv->newHandle);
        free(charset);
        return NULL;
      } else if (conv->permanent) {
        *conv->handle = conv->newHandle;
        conv->newHandle = CHARSET_ICONV_NULL;
      }

      ++conv;
    }

    while (conv != convTable) {
      if ((--conv)->newHandle != CHARSET_ICONV_NULL) {
        if (*conv->handle != CHARSET_ICONV_NULL) iconv_close(*conv->handle);
        *conv->handle = conv->newHandle;
      }
    }
  }

#elif defined(__MINGW32__)

#else /* conversions */
  if (!setCharacterSet(charset)) return 0;
#endif /* conversions */

  if (currentCharset) free(currentCharset);
  return currentCharset = charset;
}

const char *
getCharset (void) {
  return setCharset(NULL);
}
