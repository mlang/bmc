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

#ifdef HAVE_ICU
#include <unicode/uchar.h>
#include <unicode/unorm.h>
#endif /* HAVE_ICU */

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif /* HAVE_ICONV_H */

#include "unicode.h"
#include "ascii.h"

wchar_t
getBaseCharacter (wchar_t character) {
#ifdef HAVE_ICU
  {
    UChar source[] = {character};
    const unsigned int resultLength = 0X10;
    UChar resultBuffer[resultLength];
    UErrorCode error = U_ZERO_ERROR;

    unorm_normalize(source, ARRAY_COUNT(source),
                    UNORM_NFD, 0,
                    resultBuffer, resultLength,
                    &error);

    if (U_SUCCESS(error)) return resultBuffer[0];
  }
#endif /* HAVE_ICU */

#ifdef HAVE_ICONV_H
  {
    static iconv_t handle = NULL;
    if (!handle) handle = iconv_open("ASCII//TRANSLIT", "WCHAR_T");

    if (handle != (iconv_t)-1) {
      char *inputAddress = (char *)&character;
      size_t inputSize = sizeof(character);
      size_t outputSize = 0X10;
      char outputBuffer[outputSize];
      char *outputAddress = outputBuffer;

      if (iconv(handle, &inputAddress, &inputSize, &outputAddress, &outputSize) != (size_t)-1)
        if ((outputAddress - outputBuffer) == 1)
          return outputBuffer[0] & 0XFF;
    }
  }
#endif /* HAVE_ICONV_H */

  return 0;
}
