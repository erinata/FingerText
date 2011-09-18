//This file is part of ErinataNppPluginTemplate, a notepad++ snippet plugin.
//
//ErinataNppPluginTemplate is released under MIT License.
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

#ifndef NPPAPIUTILS_H
#define NPPAPIUTILS_H

#include "PluginDefinition.h"

// Connecting to Scintilla
HWND getCurrentScintilla(int which = -1);
sptr_t SendScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
void updateScintilla(int which = -1);

// Setting the menu
ShortcutKey* setShortCutKey(bool _isAlt, bool _isCtrl, bool _isShift, UCHAR _key);
int setCommand(TCHAR *cmdName = TEXT(""), PFUNCPLUGINCMD pFunc = NULL, ShortcutKey *sk = NULL, bool check0nInit = false);

// Show messageboxes
int showMessageBox(TCHAR* text = TEXT(""), int flags = MB_OK);

// Search in Scintilla
int searchNext(char* searchText, bool regExp = false);
int searchPrev(char* searchText, bool regExp = false);

// Getting Text from Scintilla
unsigned int sciGetText(char **text, int start = -1, int end = -1);
//unsigned int sciGetText(HWND hwnd, char **text, int start, int end)

void closeTab(TCHAR* path);
void openTab(TCHAR* path);
void emptyFile(TCHAR* fileName);

#endif //NPPAPIUTILS_H