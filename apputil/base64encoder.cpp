#include "base64encoder.h"
#include <assert.h>

namespace {

char const m_base64EncodeChars[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

signed char const m_base64DecodeChars[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

}

namespace sV2 {

char* decodeBase64InPlace(char* begin, char* end){
   int c1, c2, c3, c4;
   char const* g = begin; // get ptr
   char* p = begin; // put ptr
   while(g != end) {
      do {
         c1 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c1 == -1);
      if (c1 == -1) {
         break;
      }

      do {
         c2 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c2 == -1);
      if (c2 == -1) {
         break;
      }

      *p++ = char((c1 << 2) | (c2 & 0x30) >> 4);

      do {
         if (*g == '=') {
            goto done;
         }
         c3 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c3 == -1);
      if (c3 == -1) {
         break;
      }

      *p++ = char(((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2));

      do {
         if (*g == '=') {
            goto done;
         }
         c4 = m_base64DecodeChars[*g++ & 0xff];
      } while(g != end && c4 == -1);
      if (c4 == -1) {
         break;
      }

      *p++ = char(((c3 & 0x03) << 6) | c4);
   }

done:
   assert(p <= end);
   *p = '\0';
   return p;
}

std::string toBase64(char const* begin, char const* end){
   std::string out;

   char const* g = begin;
   while (g != end) {
      int c1 = *g++ & 0xff;
      if (g == end) {
         out += m_base64EncodeChars[c1 >> 2];
         out += m_base64EncodeChars[(c1 & 0x03) << 4];
         out += '=';
         out += '=';
         break;
      }

      int c2 = *g++ & 0xff;
      if (g == end) {
         out += m_base64EncodeChars[c1 >> 2];
         out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
         out += m_base64EncodeChars[(c2 & 0x0F) << 2];
         out += '=';
         break;
      }

      int c3 = *g++ & 0xff;
      out += m_base64EncodeChars[c1 >> 2];
      out += m_base64EncodeChars[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
      out += m_base64EncodeChars[((c2 & 0x0F) << 2) | ((c3 & 0xC0) >> 6)];
      out += m_base64EncodeChars[c3 & 0x3F];
   }

   return out;
}

}
