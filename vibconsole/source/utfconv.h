/*
  UTF Conversion Functions

  Include this, not utf.h
*/

#ifndef UTFCONV_H__
#define UTFCONV_H__

char     *UTF16toUTF8(const char16_t *in, size_t inLen);
char16_t *UTF8toUTF16(const char     *in, size_t inLen);

#endif

// EOF

