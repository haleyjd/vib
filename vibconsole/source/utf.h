/*
  UTF conversion code 
  
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef VIBC_UTF_H__
#define VIBC_UTF_H__

#include <algorithm>

#define PLANE1_BASE    unsigned int(0x00010000)

#define H_SURROGATE(c) char16_t(char16_t(unsigned int(c) >> 10) + char16_t(0xD7C0)) 

#define L_SURROGATE(c) char16_t(char16_t(unsigned int(c) & unsigned int(0x03FF)) | char16_t(0xDC00))

#define NS_IS_HIGH_SURROGATE(u) ((unsigned int(u) & 0xFFFFFC00) == 0xD800)
#define NS_IS_LOW_SURROGATE(u)  ((unsigned int(u) & 0xFFFFFC00) == 0xDC00)
#define IS_SURROGATE(u)         ((unsigned int(u) & 0xFFFFF800) == 0xD800)

#define SURROGATE_TO_UCS4(h, l) (((unsigned int(h) & 0x03FF) << 10) + (unsigned int(l) & 0x03FF) + PLANE1_BASE)

#define UCS2_REPLACEMENT_CHAR char16_t(0xFFFD)
#define UCS_END               unsigned int(0x00110000)

class UTF8traits
{
public:
   static bool isASCII(char c) { return (c & 0x80) == 0x00; }
   static bool isInSeq(char c) { return (c & 0xC0) == 0x80; }
   static bool is2byte(char c) { return (c & 0xE0) == 0xC0; }
   static bool is3byte(char c) { return (c & 0xF0) == 0xE0; }
   static bool is4byte(char c) { return (c & 0xF8) == 0xF0; }
   static bool is5byte(char c) { return (c & 0xFC) == 0xF8; }
   static bool is6byte(char c) { return (c & 0xFE) == 0xFC; }
};

class UTF8CharEnumerator
{
private:
   static bool CalcState(char c, unsigned int &ucs4, unsigned int &minUcs4, int &state)
   {
      if(UTF8traits::is2byte(c))
      {
         ucs4 = (unsigned int(c) << 6) & 0x000007C0L;
         state = 1;
         minUcs4 = 0x00000080;
      }
      else if(UTF8traits::is3byte(c))
      {
         ucs4 = (unsigned int(c) << 12) & 0x0000F000L;
         state = 2;
         minUcs4 = 0x00000800;
      }
      else if(UTF8traits::is4byte(c))
      {
         ucs4 = (unsigned int(c) << 18) & 0x001F0000L;
         state = 3;
         minUcs4 = 0x00010000;
      }
      else if(UTF8traits::is5byte(c))
      {
         ucs4 = (unsigned int(c) << 24) & 0x03000000L;
         state = 4;
         minUcs4 = 0x00200000;
      }
      else if(UTF8traits::is6byte(c))
      {
         ucs4 = (unsigned int(c) << 30) & 0x40000000L;
         state = 5;
         minUcs4 = 0x04000000;
      }
      else
      {
         return false;
      }

      return true;
   }

   static bool AddByte(char c, int state, unsigned int &ucs4)
   {
      if(UTF8traits::isInSeq(c))
      {
         int shift = state * 6;
         ucs4 |= (unsigned int(c) & 0x3F) << shift;
         return true;
      }
      return false;
   }

public:
   static unsigned int NextChar(const char **buffer, const char *end, bool *err)
   {
      const char *p = *buffer;
      *err = false;

      if(p >= end)
      {
         *err = true;
         return 0;
      }

      char c = *p++;

      if(UTF8traits::isASCII(c))
      {
         *buffer = p;
         return c;
      }

      unsigned int ucs4;
      unsigned int minUcs4;
      int          state = 0;

      if(!CalcState(c, ucs4, minUcs4, state))
      {
         // Not a UTF-8 string
         *err = true;
         return 0;
      }

      while(state--)
      {
         if(p == end)
         {
            *err = true;
            return 0;
         }

         c = *p++;

         if(!AddByte(c, state, ucs4))
         {
            *err = true;
            return 0;
         }
      }

      if(ucs4 < minUcs4)
      {
         // overlong sequence
         ucs4 = UCS2_REPLACEMENT_CHAR;
      }
      else if(ucs4 >= 0xD800 && (ucs4 <= 0xDFFF || ucs4 >= UCS_END))
      {
         // surrogates and code points outside the Unicode range
         ucs4 = UCS2_REPLACEMENT_CHAR;
      }

      *buffer = p;
      return ucs4;
   }
};

class ConvertUTF8toUTF16
{
public:
   typedef char value_type;
   typedef char16_t buffer_type;

private:
   buffer_type *const mStart;
   buffer_type *mBuffer;
   bool mErrorEncountered;

public:
   ConvertUTF8toUTF16(buffer_type *aBuffer)
      : mStart(aBuffer), mBuffer(aBuffer), mErrorEncountered(false)
   {
   }

   size_t Length() const { return mBuffer - mStart; }

   bool ErrorEncountered() const { return mErrorEncountered; }

   void write(const value_type *start, unsigned int N)
   {
      if(mErrorEncountered)
         return;

      // algorithm assumes utf8 units won't be spread across fragments
      const value_type *p   = start;
      const value_type *end = start + N;
      buffer_type      *out = mBuffer;

      for(; p != end; )
      {
         bool err;
         unsigned int ucs4 = UTF8CharEnumerator::NextChar(&p, end, &err);

         if(err)
         {
            mErrorEncountered = true;
            mBuffer = out;
            return;
         }

         if(ucs4 >= PLANE1_BASE)
         {
            *out++ = (buffer_type)H_SURROGATE(ucs4);
            *out++ = (buffer_type)L_SURROGATE(ucs4);
         }
         else
         {
            *out++ = ucs4;
         }
      }
      mBuffer = out;
   }

   void write_terminator()
   {
      *mBuffer = buffer_type(0);
   }
};

// Compute length of UTF16 string equivalent to a UTF8 string
class CalculateUTF8Length
{
public:
   typedef char value_type;

private:
   size_t mLength;
   bool mErrorEncountered;

public:
   CalculateUTF8Length() : mLength(0), mErrorEncountered(false)
   {
   }

   size_t Length() const { return mLength; }

   void write(const value_type *start, size_t N)
   {
      if(mErrorEncountered)
         return;

      const value_type *p   = start;
      const value_type *end = start + N;

      for(; p < end; ++mLength)
      {
         if(UTF8traits::isASCII(*p))
            p += 1;
         else if(UTF8traits::is2byte(*p))
            p += 2;
         else if(UTF8traits::is3byte(*p))
            p += 3;
         else if(UTF8traits::is4byte(*p))
         {
            if(p + 4 <= end)
            {
               unsigned int c = ((unsigned int)(p[0] & 0x07)) << 6 |
                                ((unsigned int)(p[1] & 0x30));
               if(c >= 0x010 && c < 0x110)
                  ++mLength;
            }
            p += 4;
         }
         else if(UTF8traits::is5byte(*p))
            p += 5;
         else if(UTF8traits::is6byte(*p))
            p += 6;
         else
         {
            ++mLength;
            break;
         }
      }
      if(p != end)
      {
         --mLength;
         mErrorEncountered = true;
      }
   }
};

class ConvertUTF16toUTF8
{
public:
   typedef char16_t value_type;
   typedef char     buffer_type;

private:
   buffer_type *const mStart;
   buffer_type *mBuffer;

public:
   ConvertUTF16toUTF8(buffer_type *aBuffer)
      : mStart(aBuffer), mBuffer(aBuffer)
   {
   }

   size_t Size() const { return mBuffer - mStart; }

   void write(const value_type *start, size_t N)
   {
      buffer_type *out = mBuffer;

      for(const value_type *p = start, *end = start + N; p < end; ++p)
      {
         value_type c = *p;
         if(!(c & 0xFF80))
         {
            *out++ = (char)c;
         }
         else if(!(c & 0xF800))
         {
            *out++ = 0xC0 | (char)(c >> 6);
            *out++ = 0x80 | (char)(0x003F & c);
         }
         else if(!IS_SURROGATE(c))
         {
            *out++ = 0xE0 | (char)(c >> 12);
            *out++ = 0x80 | (char)(0x003F & (c >> 6));
            *out++ = 0x80 | (char)(0x003F & c);
         }
         else if(NS_IS_HIGH_SURROGATE(c))
         {
            value_type h = c;

            ++p;
            if(p == end)
            {
               *out++ = '\xEF';
               *out++ = '\xBF';
               *out++ = '\xBD';
               break;
            }
            c = *p;
            if(NS_IS_LOW_SURROGATE(c))
            {
               unsigned int ucs4 = SURROGATE_TO_UCS4(h, c);

               *out++ = 0xF0 | (char)(ucs4 >> 18);
               *out++ = 0x80 | (char)(0x003F & (ucs4 >> 12));
               *out++ = 0x80 | (char)(0x003F & (ucs4 >> 6));
               *out++ = 0x80 | (char)(0x003F & ucs4);
            }
            else
            {
               *out++ = '\xEF';
               *out++ = '\xBF';
               *out++ = '\xBD';

               --p;
            }
         }
         else
         {
            *out++ = '\xEF';
            *out++ = '\xBF';
            *out++ = '\xBD';
         }
      }
      mBuffer = out;
   }

   void write_terminator()
   {
      *mBuffer = buffer_type(0);
   }
};

// Calculate number of bytes a UTF16 string would occupy in UTF8
class CalculateUTF8Size
{
public:
   typedef char16_t value_type;

private:
   size_t mSize;

public:
   CalculateUTF8Size() : mSize(0)
   {
   }

   size_t Size() const { return mSize; }

   void write(const value_type *start, size_t N)
   {
      for(const value_type *p = start, *end = start + N; p < end; ++p)
      {
         value_type c = *p;
         if(!(c & 0xFF80))
            mSize += 1;
         else if(!(c & 0xF800))
            mSize += 2;
         else if(0xD800 != (0xF800 & c))
            mSize += 3;
         else if(0xD800 == (0xFC00 & c))
         {
            ++p;
            if(p == end)
            {
               mSize += 3;
               break;
            }
            c = *p;

            if(0xDC00 == (0xFC00 & c))
               mSize += 4;
            else
            {
               mSize += 3;
               --p;
            }
         }
         else
         {
            mSize += 3;
         }
      }
   }
};

template<class CharT> class nsReadingIterator
{
public:
   typedef nsReadingIterator<CharT>  self_type;
   typedef ptrdiff_t                 difference_type;
   typedef CharT                     value_type;
   typedef const CharT              *pointer;
   typedef const CharT              &reference;

private:
   const CharT *mStart;
   const CharT *mEnd;
   const CharT *mPosition;

public:
   nsReadingIterator() 
   {
   }

   self_type &BeginReading(const CharT *aStart, size_t aLen)
   {
      mStart    = aStart;
      mEnd      = aStart + aLen;
      mPosition = mStart;

      return *this;
   }

   self_type &EndReading(const CharT *aStart, size_t aLen)
   {
      mStart    = aStart;
      mEnd      = aStart + aLen;
      mPosition = mEnd;

      return *this;
   }

   inline void normalize_forward()  {}
   inline void normalize_backward() {}

   pointer start() const { return mStart;    }
   pointer end()   const { return mEnd;      }
   pointer get()   const { return mPosition; }
   
   CharT operator*() const { return *get(); }

   self_type& operator++()
   {
      ++mPosition;
      return *this;
   }

   self_type operator++(int)
   {
      self_type result(*this);
      ++mPosition;
      return result;
   }

   self_type& operator--()
   {
      --mPosition;
      return *this;
   }

   self_type operator--(int)
   {
      self_type result(*this);
      --mPosition;
      return result;
   }

   difference_type size_forward() const
   {
      return mEnd - mPosition;
   }

   difference_type size_backward() const
   {
      return mPosition - mStart;
   }

   self_type &advance(difference_type n)
   {
      if (n > 0)
      {
         difference_type step = std::min(n, size_forward());
         mPosition += step;
      }
      else if (n < 0)
      {
         difference_type step = std::max(n, -size_backward());
         mPosition += step;
      }
      return *this;
   }
};

template <class CharT> struct nsCharTraits {};

template <> struct nsCharTraits<char16_t>
{
   typedef char16_t        char_type;
   typedef unsigned short  unsigned_char_type;
   typedef char            incompatible_char_type;  
   
   static char_type *move(char_type* s1, const char_type* s2, size_t n)
   {
      return static_cast<char_type*>(memmove(s1, s2, n * sizeof(char_type)));
   }
};

template <> struct nsCharTraits<char>
{
   typedef char          char_type;
   typedef unsigned char unsigned_char_type;
   typedef char16_t      incompatible_char_type;
   
   static char_type* move(char_type* s1, const char_type* s2, size_t n)
   {
      return static_cast<char_type*>(memmove(s1, s2, n * sizeof(char_type)));
   }
};

template <class CharT> class nsWritingIterator
{
public:
   typedef nsWritingIterator<CharT>  self_type;
   typedef ptrdiff_t                 difference_type;
   typedef CharT                     value_type;
   typedef CharT                    *pointer;
   typedef CharT                    &reference;

private:
   CharT *mStart;
   CharT *mEnd;
   CharT *mPosition;

public:
   nsWritingIterator()
   {
   }

   pointer BeginWriting(CharT *aStart, size_t aLen)
   {
      mStart = aStart;
      mEnd   = aStart + aLen;
      mPosition = mStart;
      return mStart;
   }

   pointer EndWriting(CharT *aStart, size_t aLen)
   {
      mStart = aStart;
      mEnd   = aStart + aLen;
      mPosition = mEnd;
      return mEnd;
   }

   inline void normalize_forward()  {}
   inline void normalize_backward() {}

   pointer start() const { return mStart;    }
   pointer end()   const { return mEnd;      }
   pointer get()   const { return mPosition; }

   reference operator*() const { return *get(); }

   self_type& operator++()
   {
      ++mPosition;
      return *this;
   }

   self_type operator++( int )
   {
      self_type result(*this);
      ++mPosition;
      return result;
   }

   self_type& operator--()
   {
      --mPosition;
      return *this;
   }

   self_type operator--( int )
   {
      self_type result(*this);
      --mPosition;
      return result;
   }

   difference_type size_forward() const
   {
      return mEnd - mPosition;
   }

   difference_type size_backward() const
   {
      return mPosition - mStart;
   }

   self_type& advance( difference_type n )
   {
      if (n > 0)
      {
         difference_type step = std::min(n, size_forward());
         mPosition += step;
      }
      else if (n < 0)
      {
         difference_type step = std::max(n, -size_backward());
         mPosition += step;
      }
      return *this;
   }

   void write(const value_type* s, size_t n)
   {
      nsCharTraits<value_type>::move(mPosition, s, n);
      advance(difference_type(n));
   }
};

template<class InputIterator> struct nsCharSourceTraits
{
   typedef typename InputIterator::difference_type difference_type;

   static size_t readable_distance(const InputIterator& first, const InputIterator& last)
   {
      // assumes single fragment
      return size_t(last.get() - first.get());
   }

   static const typename InputIterator::value_type* read(const InputIterator& iter)
   {
      return iter.get();
   }

   static void advance(InputIterator& s, difference_type n)
   {
      s.advance(n);
   }
};

template<class OutputIterator> struct nsCharSinkTraits
{
   static void write(OutputIterator& iter, const typename OutputIterator::value_type* s, size_t n)
   {
      iter.write(s, n);
   }
};

template<class InputIterator, class OutputIterator> 
inline OutputIterator& copy_string(const InputIterator& first, const InputIterator& last, OutputIterator& result)
{
   typedef nsCharSourceTraits<InputIterator> source_traits;
   typedef nsCharSinkTraits<OutputIterator>  sink_traits;

   sink_traits::write(result, source_traits::read(first), source_traits::readable_distance(first, last));
   return result;
}

#endif

// EOF

