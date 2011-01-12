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

#include <iostream>
#include <fstream>
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
void replaceTag(HWND &curScintilla, std::ifstream &file, int curPos)
{    
  std::streamoff sniplength;
   
  file.seekg(0, std::ios::end);
  sniplength = file.tellg();
  file.seekg(0, std::ios::beg);

  char* snip = new char[sniplength];
    
  file.read(snip,sniplength);
  file.close();

  ::SendMessage(curScintilla, SCI_INSERTTEXT, curPos, (LPARAM)"________`[SnippetInserting]");
  ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)snip);
      
  ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
  ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"`[SnippetInserting]");
  int aPos;
  aPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
  aPos=aPos+19;
        
  ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
  ::SendMessage(curScintilla, SCI_SEARCHPREV, 0,(LPARAM)"[>END<]");
  int bPos;
  bPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));

  ::SendMessage(curScintilla, SCI_SETSELECTIONSTART, bPos,(LPARAM)true);
  ::SendMessage(curScintilla, SCI_SETSELECTIONEND, aPos,(LPARAM)true);
                
  ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");

}



void fingerText()
{
    int tagFound = 0;
	// Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	  int curPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
	  
    ::SendMessage(curScintilla,SCI_WORDLEFTEXTEND,0,0);

    int curPos2= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));

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
    
    TCHAR lang[20]=L"snippet";
    ::wcscat(lang,ext);
    int found;
    found=static_cast<int>(::SetCurrentDirectory(lang));
    
    if (found<=0) ::SetCurrentDirectory(L"snippet.global");

    std::ifstream file;
    file.open(tag);

    if (file.is_open())
    {
      tagFound = 1;
      replaceTag(curScintilla, file, curPos);
      
    } else if(found>0)
    {
      //::SetCurrentDirectory(L"..");
      ::SetCurrentDirectory(L"..\\snippet.global");
      file.open(tag);
      if (file.is_open())
      {
        tagFound = 1;
        replaceTag(curScintilla, file, curPos);
      } 
    }

    // return to the original path 
    ::SetCurrentDirectory(curPath);
    // return to the original position 
    ::SendMessage(curScintilla,SCI_GOTOPOS,curPos2,0);
    

 // This is the part doing Hotspots tab navigation
   
	  int firstPos;
	  int secondPos;
	  int result;
	
	  ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
	  result=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"$[![");
	  if (result>=0)
	  {
		  firstPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
		  ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
		  ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"]!]");
		  secondPos= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
		  secondPos=secondPos+3;

		  ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,firstPos,0);
		  ::SendMessage(curScintilla,SCI_SETSELECTIONEND,secondPos,0);
	  } else if (tagFound == 0)
	  {
      ::SendMessage(curScintilla,SCI_GOTOPOS,curPos,0);
      
      ::SendMessage(curScintilla,SCI_TAB,0,0);		  
	  }/*
    */
}


