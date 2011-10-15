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

#include "ConversionUtils.h"

std::string toString(const char &c)
{
    std::stringstream ss;
    ss << c;
    return ss.str();
}

std::string toString(const double &d) 
{
    std::stringstream ss;
    ss << d;
    return ss.str();
}

std::string toString(int &i) 
{
    std::stringstream ss;
    ss << i;
    return ss.str();
}

std::string toString(char* source)
{
    std::stringstream ss;
    ss << source;
    return ss.str();
}


wchar_t* toWideChar(char* orig)
{
    wchar_t* wideChar;
    if (orig == NULL)
    {
        wideChar = new wchar_t[1];
        ::wcscpy(wideChar,TEXT("\0"));
    } else
    {
        size_t origsize = strlen(orig) + 1;
        size_t convertedChars = 0;
        wideChar = new wchar_t[origsize*4+1];
        mbstowcs_s(&convertedChars, wideChar, origsize, orig, _TRUNCATE);
    }
    return wideChar;
}

wchar_t* toWideChar(std::string source)
{
    char* temp = new char[source.length()+1];
    strcpy(temp,source.c_str());
    wchar_t* wide = toWideChar(temp);
    delete [] temp;
    return wide;
}


char* toCharArray(std::string source)
{
    if (source.length()<=0) source = "";
    char* dest = new char [source.length()+1];
    strcpy(dest, source.c_str());
    return dest;
}

char* toCharArray(TCHAR *orig, UINT codePage)
{
    char* mb;
    if (orig == NULL)
    {
        mb = new char[1];
        mb[0] = '\0';
    } else
    {
        //alert((int)::SendScintilla(SCI_GETCODEPAGE, 0, 0));
	    int multibyteLength = WideCharToMultiByte(codePage, 0, orig, -1, NULL, 0, 0, 0);
	    mb = new char[multibyteLength + 1];
	    WideCharToMultiByte(codePage, 0, orig, -1, mb, multibyteLength, 0, 0);
        
    }
    return mb;

}

double toDouble(const std::string &s)
{
    std::stringstream ss(s);
    double d;
    ss >> d;
    return d;
}

 
long toLong(std::string source)
{
    std::stringstream ss(source);
    long l;
    ss >> l;
    return l;
}           

std::vector<std::string> toVectorString(char* str, char c, int parts)
{
    int i = 1;
    if (parts == 0) i = -1;
        
    std::vector<std::string> result;

    while(1)
    {
        
        char *begin = str;
        while ((*str != c && *str) || ((i >= parts) && *str)) str++;
        result.push_back(std::string(begin, str));
        if (parts != 0) i++;
        if (0 == *str++) break;
    }

    return result;
}

std::vector<std::string> toSortedVectorString(std::vector<std::string> v)
{
    std::sort(v.begin(), v.end());
    return v;
}



int toVk(char* input)
{
    if (strlen(input) == 1)
    {
        char i = toupper(input[0]);
        return (int)i;
    }
    //TODO: cater lower case for other keywords

    if (strcmp(input,"BACK") == 0) return 0x08;
    else if (strcmp(input,"TAB") == 0) return 0x09;
    else if (strcmp(input,"CLEAR") == 0) return 0x0C;
    else if (strcmp(input,"RETURN") == 0) return 0x0D;
    else if (strcmp(input,"SHIFT") == 0) return 0x10;
    else if (strcmp(input,"CONTROL") == 0) return 0x11;
    else if (strcmp(input,"MENU") == 0) return 0x12;
    else if (strcmp(input,"PAUSE") == 0) return 0x13;
    else if (strcmp(input,"CAPITAL") == 0) return 0x14;
    else if (strcmp(input,"ESCAPE") == 0) return 0x1B;
    else if (strcmp(input,"SPACE") == 0) return 0x20;
    else if (strcmp(input,"PRIOR") == 0) return 0x21;
    else if (strcmp(input,"NEXT") == 0) return 0x22;
    else if (strcmp(input,"END") == 0) return 0x23;
    else if (strcmp(input,"HOME") == 0) return 0x24;
    else if (strcmp(input,"LEFT") == 0) return 0x25;
    else if (strcmp(input,"UP") == 0) return 0x26;
    else if (strcmp(input,"RIGHT") == 0) return 0x27;
    else if (strcmp(input,"DOWN") == 0) return 0x28;
    else if (strcmp(input,"SELECT") == 0) return 0x29;
    else if (strcmp(input,"EXECUTE") == 0) return 0x2B;
    else if (strcmp(input,"SNAPSHOT") == 0) return 0x2C;
    else if (strcmp(input,"INSERT") == 0) return 0x2D;
    else if (strcmp(input,"DELETE") == 0) return 0x2E;
    else if (strcmp(input,"HELP") == 0) return 0x2F;
    else if (strcmp(input,"LWIN") == 0) return 0x5B;
    else if (strcmp(input,"RWIN") == 0) return 0x5C;
    else if (strcmp(input,"APPS") == 0) return 0x5D;
    else if (strcmp(input,"NUMPAD0") == 0) return 0x60;
    else if (strcmp(input,"NUMPAD1") == 0) return 0x61;
    else if (strcmp(input,"NUMPAD2") == 0) return 0x62;
    else if (strcmp(input,"NUMPAD3") == 0) return 0x63;
    else if (strcmp(input,"NUMPAD4") == 0) return 0x64;
    else if (strcmp(input,"NUMPAD5") == 0) return 0x65;
    else if (strcmp(input,"NUMPAD6") == 0) return 0x66;
    else if (strcmp(input,"NUMPAD7") == 0) return 0x67;
    else if (strcmp(input,"NUMPAD8") == 0) return 0x68;
    else if (strcmp(input,"NUMPAD9") == 0) return 0x69;
    else if (strcmp(input,"MULTIPLY") == 0) return 0x6A;
    else if (strcmp(input,"ADD") == 0) return 0x6B;
    else if (strcmp(input,"SEPARATOR") == 0) return 0x6C;
    else if (strcmp(input,"SUBTRACT") == 0) return 0x6D;
    else if (strcmp(input,"DECIMAL") == 0) return 0x6E;
    else if (strcmp(input,"DIVIDE") == 0) return 0x6F;
    else if (strcmp(input,"F1") == 0) return 0x70;
    else if (strcmp(input,"F2") == 0) return 0x71;
    else if (strcmp(input,"F3") == 0) return 0x72;
    else if (strcmp(input,"F4") == 0) return 0x73;
    else if (strcmp(input,"F5") == 0) return 0x74;
    else if (strcmp(input,"F6") == 0) return 0x75;
    else if (strcmp(input,"F7") == 0) return 0x76;
    else if (strcmp(input,"F8") == 0) return 0x77;
    else if (strcmp(input,"F9") == 0) return 0x78;
    else if (strcmp(input,"F10") == 0) return 0x79;
    else if (strcmp(input,"F11") == 0) return 0x7A;
    else if (strcmp(input,"F12") == 0) return 0x7B;
    else if (strcmp(input,"F13") == 0) return 0x7C;
    else if (strcmp(input,"F14") == 0) return 0x7D;
    else if (strcmp(input,"F15") == 0) return 0x7E;
    else if (strcmp(input,"F16") == 0) return 0x7F;
    else if (strcmp(input,"F17") == 0) return 0x80;
    else if (strcmp(input,"F18") == 0) return 0x81;
    else if (strcmp(input,"F19") == 0) return 0x82;
    else if (strcmp(input,"F20") == 0) return 0x83;
    else if (strcmp(input,"F21") == 0) return 0x84;
    else if (strcmp(input,"F22") == 0) return 0x85;
    else if (strcmp(input,"F23") == 0) return 0x86;
    else if (strcmp(input,"F24") == 0) return 0x87;
    else if (strcmp(input,"NUMLOCK") == 0) return 0x90;
    else if (strcmp(input,"SCROLL") == 0) return 0x91;
    else if (strcmp(input,"LSHIFT") == 0) return 0xA0;
    else if (strcmp(input,"RSHIFT") == 0) return 0xA1;
    else if (strcmp(input,"LCONTROL") == 0) return 0xA2;
    else if (strcmp(input,"RCONTROL") == 0) return 0xA3;
    else if (strcmp(input,"LMENU") == 0) return 0xA4;
    else if (strcmp(input,"RMENU") == 0) return 0xA5;
    else return 0;
    //TODO: complete this list

}