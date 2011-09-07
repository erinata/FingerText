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

#include "DebugUtils.h"


void alert()
{
    alert(TEXT("Alert!"));
}

void alert(int input)
{
    std::stringstream ss;
    ss << input;
    alert(ss.str());
}

void alert(char input)
{
    char* temp = new char[2];
    temp[0] = input;
    temp[1] = '\0';

    alert(temp);
    delete [] temp;

}

void alert(char* input)
{
    wchar_t* temp;

    if (input == NULL)
    {
        temp = NULL;
    } else
    {
        size_t origsize = strlen(input) + 1;
        size_t convertedChars = 0;
        temp = new wchar_t[origsize*4+1];
        
        mbstowcs_s(&convertedChars, temp, origsize, input, _TRUNCATE);
        
    }
    alert(temp);
    delete [] temp;
}

void alert(TCHAR* input)
{
    ::MessageBox(NULL, input, TEXT("DEBUG"), MB_OK);
}

void alert(std::string input)
{
    char* temp;

    if (input.length()<=0) input = "";
    temp = new char [input.length()+1];
    strcpy(temp, input.c_str());

    alert(temp);
    delete [] temp;
}

void alert(std::vector<std::string> input)
{
    int i = 0;
    while (i<input.size())
    {
        alert(input[i]);
        i++;
    }
}

void alert(std::vector<int> input)
{
    int i = 0;
    while (i<input.size())
    {
        alert(input[i]);
        i++;
    }
}

void alert(HWND input)
{

    long handle = reinterpret_cast<long>(input);
    alert(handle);
}

void alert(double input)
{
    std::stringstream ss;
    ss << input;
    alert(ss.str());
}