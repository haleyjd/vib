/*
  UTF Conversion Code

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <stdlib.h>
#include "utf.h"

char *UTF16toUTF8(const char16_t *in, size_t inLen)
{
   char *out = nullptr;
   nsReadingIterator<char16_t> source_start, source_end;
   CalculateUTF8Size calculator;
   copy_string(source_start.BeginReading(in, inLen), source_end.EndReading(in, inLen), calculator);
   
   size_t count = calculator.Size();

   if(count)
   {
      out = new char [count+1];

      nsWritingIterator<char> dest;
      ConvertUTF16toUTF8 converter(dest.BeginWriting(out, (unsigned int)count));
      copy_string(source_start.BeginReading(in, inLen), source_end.EndReading(in, inLen), converter);
      converter.write_terminator();
   }

   return out;
}

char16_t *UTF8toUTF16(const char *in, size_t inLen)
{
   char16_t *out = nullptr;
   nsReadingIterator<char> source_start, source_end;
   CalculateUTF8Length calculator;
   copy_string(source_start.BeginReading(in, inLen), source_end.EndReading(in, inLen), calculator);

   size_t count = calculator.Length();

   if(count)
   {
      out = new char16_t [count + 1];

      nsWritingIterator<char16_t> dest;
      ConvertUTF8toUTF16 converter(dest.BeginWriting(out, (unsigned int)count));
      copy_string(source_start.BeginReading(in, inLen), source_end.EndReading(in, inLen), converter);
      converter.write_terminator();
   }

   return out;
}

// EOF

