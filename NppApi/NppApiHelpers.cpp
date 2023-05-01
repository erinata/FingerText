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

#include "NppApiHelpers.h"

extern FuncItem funcItem[MENU_LENGTH];
extern NppData nppData;

SciFnDirect pSciMsg;  // For direct scintilla call
sptr_t pSciWndData;   // For direct scintilla call

sptr_t SendScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return pSciMsg(pSciWndData, iMessage, wParam, lParam);
}

HWND getCurrentScintilla(int which)
{
    /* 
        which = -1 (default) return the current scintilla handle
        which = 0 return the main scintilla handle
        which = 1 return the second scintilla handle
        which = 2 return the non-current scintilla handle
    */

    if (which == -1)
    {
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    } else if (which == 2)
    {
        ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
        which = 1 - which;
    }

    if (which == 0)
    {
        return nppData._scintillaMainHandle;
    } else if (which == 1)
    {
        return nppData._scintillaSecondHandle;
    } else
    {
        return NULL;
    }

    return nppData._scintillaMainHandle;
}

void updateScintilla(int which, HWND curScintilla)
{
    if (curScintilla == NULL) curScintilla = getCurrentScintilla(which);
    pSciMsg = (SciFnDirect)SendMessage(curScintilla,SCI_GETDIRECTFUNCTION, 0, 0);
    pSciWndData = (sptr_t)SendMessage(curScintilla,SCI_GETDIRECTPOINTER, 0, 0);
}


ShortcutKey* setShortCutKey(bool _isAlt, bool _isCtrl, bool _isShift, UCHAR _key)
{
    ShortcutKey *shKey = new ShortcutKey;
	shKey->_isAlt = _isAlt;
	shKey->_isCtrl = _isCtrl;
	shKey->_isShift = _isShift;
	shKey->_key = _key;
    return shKey;
}

int setCommand(TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    static int cmdIndex = 0;
    if (cmdIndex >= MENU_LENGTH) return cmdIndex;
    if (!pFunc) return cmdIndex++;

    lstrcpy(funcItem[cmdIndex]._itemName, cmdName);
    funcItem[cmdIndex]._pFunc = pFunc;
    funcItem[cmdIndex]._init2Check = check0nInit;
    funcItem[cmdIndex]._pShKey = sk;

    return cmdIndex++;
}

int showMessageBox(TCHAR* text, int flags)
{
    return ::MessageBox(nppData._nppHandle, text, TEXT(PLUGIN_NAME), flags);
}

sptr_t searchNext(char* searchText, bool regExp)
{
    int searchFlags = 0;
    if (regExp) searchFlags = SCFIND_REGEXP;
    ::SendScintilla(SCI_SEARCHANCHOR, 0,0);
    return ::SendScintilla(SCI_SEARCHNEXT, searchFlags,(LPARAM)searchText);
}

sptr_t searchPrev(char* searchText, bool regExp)
{    
    int searchFlags = 0;
    if (regExp) searchFlags = SCFIND_REGEXP;
    ::SendScintilla(SCI_SEARCHANCHOR, 0,0); 
    return ::SendScintilla(SCI_SEARCHPREV, searchFlags,(LPARAM)searchText);
}


sptr_t sciGetText(char **text, sptr_t start, sptr_t end)
{
    if (start == -1)
    {
        start = SendScintilla(SCI_GETSELECTIONSTART, 0, 0);
        end = SendScintilla(SCI_GETSELECTIONEND, 0, 0);
    }

    *text = (LPSTR)new char[end - start + 1];

    if (end > start)
    {

        TextRange tr;
        tr.chrg.cpMin = start;
        tr.chrg.cpMax = end;
        tr.lpstrText  = *text;
        return SendScintilla(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
    } else
    {
        strcpy(*text,"");
        return 0;
    }
}


//unsigned int sciGetText(HWND hwnd, char **text, int start, int end)
//{
//    *text = (LPSTR)new char[end - start + 1];
//    TextRange tr;
//    tr.chrg.cpMin = start;
//    tr.chrg.cpMax = end;
//    tr.lpstrText  = *text;
//    if (end > start)
//    {
//        return  (int)::SendMessage(hwnd, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
//    } else
//    {
//        strcpy(*text,"");
//        return 0;
//    }
//
//    // With this implementation we can specify the handle of the text to cut, for example,
//    //ScintillaGetText(nppData._scintillaMainHandle, buffer, start, end);
//    //ScintillaGetText(nppData._scintillaSecondHandle, buffer, start, end);
//}
//

void closeTab(TCHAR* path)
{
    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)path))
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
    } 
}

void openTab(TCHAR* path)
{
    if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)path))
    {
        ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)path);
    } 
}

void emptyFile(TCHAR* fileName)
{
    std::ofstream File;
    File.open(fileName,std::ios::out|std::ios::trunc);
    File.close();
}

