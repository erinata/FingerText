//This file is part of EriniataNppPluginTemplate, a notepad++ snippet plugin.
//
//EriniataNppPluginTemplate is released under MIT License.
//
//MIT license
//
//Copyright (C) 2011 by Tom Lam
//
//Permission is hereby granted, free of charge, to any person 
//obtaining a copy of this software and associated documentation 
//files (the "Software"), to deal in the Software without 
//restriction, including without limitation the rights to use, 
//copy, modify, merge, publish, distribute, sublicense, and/or 
//sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following 
//conditions:
//
//The above copyright notice and this permission notice shall be 
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//DEALINGS IN THE SOFTWARE.

#include "StringUtils.h"


void findAndReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

char* quickStrip(char *str, char key)
{
    if (str==NULL) return NULL;

    char *from, *to;
    from=to=str;

    while ((*from != key) && (*to++=*from),*from++);
    return str;
}



char *replaceAll(char *src, const char *fromstr, const char *tostr) {
    char *result, *sr;
    size_t i, count = 0;
    size_t fromlen = strlen(fromstr); if (fromlen < 1) return src;
    size_t tolen = strlen(tostr);

    if (tolen != fromlen) {
        for (i = 0; src[i] != '\0';) {
            if (memcmp(&src[i], fromstr, fromlen) == 0) count++, i += fromlen;
        else i++;
        }
    } else i = strlen(src);


    result = (char *) malloc(i + 1 + count * (tolen - fromlen));
    if (result == NULL) return NULL;


    sr = result;
    while (*src) {
        if (memcmp(src, fromstr, fromlen) == 0) {
            memcpy(sr, tostr, tolen);
            sr += tolen;
            src  += fromlen;
        } else *sr++ = *src++;
    }
    *sr = '\0';

    return result;
}