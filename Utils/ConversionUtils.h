//This file is part of FingerText, a notepad++ snippet plugin.
//
//FingerText is released under MIT License.
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

#ifndef CONVERSIONUTILS_H
#define CONVERSIONUTILS_H

#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
//#include <tchar.h>

std::string toString(const char &c);
std::string toString(const double &d);
std::string toString(int &i);
std::string toString(char* source);


wchar_t* toWideChar(char* orig);

char* toCharArray(std::string source);
char* toCharArray(TCHAR *orig, UINT codePage  = CP_UTF8);

double toDouble(const std::string &s);

long toLong(std::string source);

std::vector<std::string> toVectorString(char* str, char c = ' ', int parts = 0);
//std::vector<std::string> split3(char* str, char* c, int parts);
//std::vector<std::string> split2(char* str, char c1, char c2, char c3);
std::vector<std::string> toSortedVectorString(std::vector<std::string> v);

int toVk(char* input);


#endif //CONVERSIONUTILS_H