//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"



//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    ShortcutKey *shKey = new ShortcutKey;
	  shKey->_isAlt = false;
	  shKey->_isCtrl = false;
	  shKey->_isShift = false;
	  shKey->_key = VK_TAB;
    
    setCommand(0, TEXT("Trigger FingerText"), fingerText, shKey, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
  delete funcItem[0]._pShKey;
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//




void fingerText()
{
    
	// Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    int rectangleSelction = static_cast<int>(::SendMessage(curScintilla,SCI_SELECTIONISRECTANGLE,0,0));

    if (rectangleSelction==1)
    {
      ::SendMessage(curScintilla,SCI_TAB,0,0);	
    } else
    {
      int tagFound = 0;
    
	    int posCurrent= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
      int posSelectionStart= static_cast<int>(::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0));
      int posSelectionEnd= static_cast<int>(::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0));

      if (posSelectionStart==posSelectionEnd)
      {
        ::SendMessage(curScintilla,SCI_WORDLEFTEXTEND,0,0);

        int posBeforeTag= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));

        char tag[256];
	      ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&tag);
        TCHAR curPath[MAX_PATH];
        ::GetCurrentDirectory(MAX_PATH,(LPTSTR)curPath);
    
        TCHAR path[MAX_PATH];
        ::SendMessage(nppData._nppHandle, NPPM_GETNPPDIRECTORY, (WPARAM)MAX_PATH, (LPARAM)path);

        TCHAR ext[10];
        ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)ext);
    
        ::wcscat(path,L"\\plugins\\FingerText\\");
        ::SetCurrentDirectory(path);
    
        TCHAR lang[20]=L"(snippet)";
        ::wcscat(lang,ext);
    
        int folderFound=static_cast<int>(::SetCurrentDirectory(lang));
    
        if (folderFound<=0) ::SetCurrentDirectory(L"(snippet).global");

        std::ifstream file;
        file.open(tag);

        if (file.is_open())
        {
          tagFound = 1;
          replaceTag(curScintilla, file, posCurrent);
      
        } else if(folderFound>0)
        {
          //::SetCurrentDirectory(L"..");
          ::SetCurrentDirectory(L"..\\(snippet).global");
          file.open(tag);
          if (file.is_open())
          {
            tagFound = 1;
            replaceTag(curScintilla, file, posCurrent);
          } 
        }

        // return to the original path 
        ::SetCurrentDirectory(curPath);
        // return to the original position 
        ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);

      }
	  
   // This is the part doing Hotspots tab navigation
   
	
	    ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
	    int spotFound=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"$[![");
	    if (spotFound>=0)
	    {
		    int firstPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
		    ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
		    ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"]!]");
		    int secondPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0))+3;

		    ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,firstPos,0);
		    ::SendMessage(curScintilla,SCI_SETSELECTIONEND,secondPos,0);
	    } else if (tagFound == 0)
	    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent,0);
        ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,posSelectionStart,0);
        ::SendMessage(curScintilla,SCI_SETSELECTIONEND,posSelectionEnd,0);
        ::SendMessage(curScintilla,SCI_TAB,0,0);	
      }
	  } 
}

void replaceTag(HWND &curScintilla, std::ifstream &file, int pos)
{    
  //std::streamoff sniplength;
  int sniplength;

  file.seekg(0, std::ios::end);
  sniplength = file.tellg();
  file.seekg(0, std::ios::beg);

  char* snip = new char[sniplength*2];
    
  file.read(snip,sniplength);
  file.close();

  ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

  ::SendMessage(curScintilla, SCI_INSERTTEXT, pos, (LPARAM)"_____________________________`[SnippetInserting]");

  
  // Failed attempt to cater unicode snippets
  //if (::IsTextUnicode(snip,sniplength,0))
  //{
  //  ::MessageBox(nppData._nppHandle, TEXT("ANSI"), TEXT("Trace"), MB_OK);
  //} else
  //{
  //  ::MessageBox(nppData._nppHandle, TEXT("not ANSI"), TEXT("Trace"), MB_OK);
  //}

  // Just assume that all snippets are in ANSI, and convert to UTF-8 when needed.
  if (::SendMessage(curScintilla,SCI_GETCODEPAGE,0,0)==65001)
  {
    //::MessageBox(nppData._nppHandle, TEXT("65001"), TEXT("Trace"), MB_OK);
    WCHAR *w=new WCHAR[sniplength*2];
    MultiByteToWideChar(CP_ACP, 0, snip, -1, w, sniplength*2); // ANSI to UNICODE
    WideCharToMultiByte(CP_UTF8, 0, w, -1, snip, sniplength*2, 0, 0); // UNICODE to UTF-8
    delete [] w;
  }
  
  ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)snip);
      
  ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
  ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"`[SnippetInserting]");
  int posEndOfInsertedText= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0))+19;
        
  ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
  ::SendMessage(curScintilla, SCI_SEARCHPREV, 0,(LPARAM)"[>END<]");
  int posEndOfSnippet= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));

  ::SendMessage(curScintilla, SCI_SETSELECTIONSTART, posEndOfSnippet,(LPARAM)true);
  ::SendMessage(curScintilla, SCI_SETSELECTIONEND, posEndOfInsertedText,(LPARAM)true);
                
  ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");

  
  //delete [] snip; // This cause problem when we trigger a long snippet and then a short snippet
  ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);

}