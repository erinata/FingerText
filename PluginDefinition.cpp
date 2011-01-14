//This file is part of the notepad++ plugin FingerText. This file 
//is modified from the NppPluginTemplate provided by Don Ho. The 
//License of the template is shown below. 
//
/////////////////////////////////////////////////////
//
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
//void convertEncoding(char tag[60],UINT codePageFrom,UINT codePageTo)
//{
//    ::MessageBox(nppData._nppHandle, TEXT("converted"), TEXT("Trace"), MB_OK);
//    WCHAR *w=new WCHAR[120];
//    MultiByteToWideChar(codePageFrom, 0, tag, -1, w, 120); 
//    WideCharToMultiByte(codePageTo, 0, w, -1, tag, 120, 0, 0); 
//    delete [] w;
//}



void fingerText()
{
    //::Sleep(10);
	// Get the current scintilla
    HWND curScintilla = getCurrentScintilla();
    

    if (::SendMessage(curScintilla,SCI_SELECTIONISRECTANGLE,0,0)==1)
    {
        ::SendMessage(curScintilla,SCI_TAB,0,0);	
    } else
    {
        int tagFound = 0;

        int posBeforeTag=0;
    
	    int posCurrent= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        int posSelectionStart= ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
        int posSelectionEnd= ::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0);

        if (posSelectionStart!=posSelectionEnd)
        {
            //::MessageBox(nppData._nppHandle, TEXT("selection"), TEXT("Trace"), MB_OK);
        } else
        {
                       
            ::SendMessage(curScintilla,SCI_WORDLEFTEXTEND,0,0);
            posBeforeTag= static_cast<int>(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));

            if (posCurrent-posBeforeTag>40)
            {
                //::MessageBox(nppData._nppHandle, TEXT("long tag"), TEXT("Trace"), MB_OK); 
                // Still contain some issues like if we tab on a position where there are a lot of tab spaces before that,
                // those tabs spaces will be reduced for no reason.
            } else
            {
                char tag[60];
	            ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&tag);

                //::MessageBox(nppData._nppHandle, (LPCWSTR)tag, TEXT("Trace"), MB_OK);
                // Here the tag is got assuming the document is in ANSI, if the document is in UTF-8,
                // chinese character tag is not loaded
                if (::SendMessage(curScintilla,SCI_GETCODEPAGE,0,0)==65001)
                {
                    //::MessageBox(nppData._nppHandle, TEXT("65001"), TEXT("Trace"), MB_OK);
                    //convertEncoding(tag,CP_UTF8,CP_ACP);
                    WCHAR *w=new WCHAR[120];
                    MultiByteToWideChar(CP_UTF8, 0, tag, -1, w, 120); 
                    WideCharToMultiByte(CP_ACP, 0, w, -1, tag, 120, 0, 0); 
                    delete [] w;
                }
  
                TCHAR curPath[MAX_PATH];
                ::GetCurrentDirectory(MAX_PATH,(LPTSTR)curPath);
    
                TCHAR path[MAX_PATH];
                ::SendMessage(nppData._nppHandle, NPPM_GETNPPDIRECTORY, (WPARAM)MAX_PATH, (LPARAM)path);
                                
                ::wcscat(path,L"\\plugins\\FingerText\\");
                ::SetCurrentDirectory(path);

                //
                //TCHAR namePart[30];
                //::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)namePart);
                //
                //TCHAR fileName[40]=L"(snippet)";
                //::wcscat(fileName,namePart);

                TCHAR extPart[10];
                ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)extPart);
    
                TCHAR langName[20]=L"(snippet)";
                ::wcscat(langName,extPart);

                int folderFound=static_cast<int>(::SetCurrentDirectory(langName));
    
                if (folderFound<=0) ::SetCurrentDirectory(L"(snippet).global");

                std::ifstream file;
                file.open(tag);

                if (file.is_open())
                {
                    tagFound = replaceTag(curScintilla, file, posCurrent,posBeforeTag);
      
                } else if(folderFound>0)
                {
                    //::SetCurrentDirectory(L"..");
                    ::SetCurrentDirectory(L"..\\(snippet).global");
                    file.open(tag);
                    if (file.is_open())
                    {
                        tagFound = replaceTag(curScintilla, file, posCurrent,posBeforeTag);
                    } 
                }
                // return to the original path 
                ::SetCurrentDirectory(curPath);
            }
            // return to the original position 
            ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
        }
	  
        int spotFound = hotSpotNavigation(curScintilla);

        if ((spotFound==0) && (tagFound == 0)) restoreTab(curScintilla, posCurrent, posSelectionStart, posSelectionEnd);
    } 

}


HWND getCurrentScintilla()
{
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return NULL;
    if (which == 0)
    {
        return nppData._scintillaMainHandle;
    } else
    {
        return nppData._scintillaSecondHandle;
    }
    return nppData._scintillaMainHandle;
}

void restoreTab(HWND &curScintilla, int &posCurrent, int &posSelectionStart, int &posSelectionEnd)
{
    // restoring the original tab action
    ::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent,0);
    ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,posSelectionStart,0);
    ::SendMessage(curScintilla,SCI_SETSELECTIONEND,posSelectionEnd,0);
    ::SendMessage(curScintilla,SCI_TAB,0,0);	
}

int hotSpotNavigation(HWND &curScintilla)
{
    int preserveSteps=0;
    // This is the part doing Hotspots tab navigation
    
    ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
	int spot=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"$[![");

	if (spot>=0)
	{
        if (preserveSteps==0) ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
        //::MessageBox(nppData._nppHandle, TEXT(">=0"), TEXT("Trace"), MB_OK);
		int firstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        int posLine = ::SendMessage(curScintilla,SCI_LINEFROMPOSITION,0,0);
        
		::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
		::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"]!]");
		int secondPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

        ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,firstPos+4,0);
		::SendMessage(curScintilla,SCI_SETSELECTIONEND,secondPos,0);
        
        char hotSpotText[120];
        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&hotSpotText);
        ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,firstPos,0);
		::SendMessage(curScintilla,SCI_SETSELECTIONEND,secondPos+3,0);
        
        char hotSpot[120];
        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&hotSpot);
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)&hotSpotText);
        ::SendMessage(curScintilla,SCI_GOTOPOS,secondPos+3,0);
        
        int hotSpotFound=-1;
        int tempPos[60];

        int i=1;

        for (i=1;i<=50;i++)
        {
            tempPos[i]=0;
        
            ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
            hotSpotFound=::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)hotSpot);
            if (hotSpotFound>=0)
            {
                //::MessageBox(nppData._nppHandle, TEXT(">=0"), TEXT("Trace"), MB_OK);
                tempPos[i] = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)&hotSpotText);
                ::SendMessage(curScintilla,SCI_GOTOPOS,tempPos[i]+1,0);
            } else
            {
                break;
                //::MessageBox(nppData._nppHandle, TEXT("<0"), TEXT("Trace"), MB_OK);
                //tempPos[i]=-1;
            }
        }

        
        //::SendMessage(curScintilla,SCI_GOTOPOS,::SendMessage(curScintilla,SCI_POSITIONFROMLINE,posLine,0),0);
        //::SendMessage(curScintilla,SCI_GOTOLINE,posLine,0);

        ::SendMessage(curScintilla,SCI_GOTOPOS,firstPos,0);
        ::SendMessage(curScintilla,SCI_SCROLLCARET,0,0);
        
        ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos-4);
        for (int j=1;j<i;j++)
        {
            if (tempPos[j]!=-1)
            {
                ::SendMessage(curScintilla,SCI_ADDSELECTION,tempPos[j],tempPos[j]+(secondPos-4-firstPos));
            }
        }
        ::SendMessage(curScintilla,SCI_SETMAINSELECTION,0,0);
        ::SendMessage(curScintilla,SCI_LINESCROLL,0,0);
        

        if (preserveSteps==0) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
        return 1;
	}
    //::MessageBox(nppData._nppHandle, TEXT("<0"), TEXT("Trace"), MB_OK);
    return 0;
}


int replaceTag(HWND &curScintilla, std::ifstream &file, int &posCurrent, int &posBeforeTag)
{
    int preserveSteps=0;
    //::MessageBox(nppData._nppHandle, TEXT("replace tag"), TEXT("Trace"), MB_OK); 
    //std::streamoff sniplength;
    int sniplength;

    file.seekg(0, std::ios::end);
    sniplength = file.tellg();
    file.seekg(0, std::ios::beg);

    if (sniplength<=1)
    {
        //::MessageBox(nppData._nppHandle, TEXT("no text"), TEXT("Trace"), MB_OK); 
        return 0;
    } else
    {
        //::MessageBox(nppData._nppHandle, TEXT("contain text"), TEXT("Trace"), MB_OK); 
        char* snip = new char[sniplength*2];
    
        file.read(snip,sniplength);
        file.close();

        if (preserveSteps==0) ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

        ::SendMessage(curScintilla, SCI_INSERTTEXT, posCurrent, (LPARAM)"_____________________________`[SnippetInserting]");

  
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
        int posEndOfInsertedText= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;
            
        ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
        ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
        ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"[>END<]");
        int posEndOfSnippet= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        
        ::SendMessage(curScintilla, SCI_SETSELECTIONSTART, posEndOfSnippet,(LPARAM)true);
        ::SendMessage(curScintilla, SCI_SETSELECTIONEND, posEndOfInsertedText,(LPARAM)true);

        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
          
        delete [] snip; 
        // This cause problem when we trigger a long snippet and then a short snippet
        // The problem is solved after using a better way to search for [>END<]
        if (preserveSteps==0) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
        return 1;
    }
}


