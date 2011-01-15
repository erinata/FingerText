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

#include <tchar.h>

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

/** buffer gets allocated in this function (if return != 0). 
 *  Resposibility of the caller to free it
 *  returns 0 if tag not found (buffer in this case not allocated)
 */
int getCurrentTag(HWND curScintilla, int posCurrent, char** buffer)
{
	int retVal = 0;
	int posBeforeTag = static_cast<int>(::SendMessage(curScintilla,	SCI_WORDSTARTPOSITION, posCurrent, 1));
    
            
    if (posCurrent - posBeforeTag < 100) // Max tag length 100
    {
        *buffer = new char[posCurrent - posBeforeTag + 1];
		Sci_TextRange range;
		range.chrg.cpMin = posBeforeTag;
		range.chrg.cpMax = posCurrent;
		range.lpstrText = *buffer;

	    ::SendMessage(curScintilla, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));
		retVal = posCurrent - posBeforeTag;
	}

	return retVal;
}


char* findTag(char *tag, TCHAR *fileType = NULL)
{
	char* snip = NULL;
	TCHAR curPath[MAX_PATH];
    ::GetCurrentDirectory(MAX_PATH,(LPTSTR)curPath);
    
    TCHAR path[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETNPPDIRECTORY, (WPARAM)MAX_PATH, (LPARAM)path);
                                
    ::wcscat(path,L"\\plugins\\FingerText\\");
    ::SetCurrentDirectory(path);

   
    TCHAR tagType[MAX_PATH];
    TCHAR tagPath[MAX_PATH];

    ::swprintf(tagPath,L"(snippet)");

	if (fileType == NULL)
	{
		::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)tagType);
		::wcscat(tagPath, tagType);
	}
	else
	{
		_tcscat(tagPath, fileType);
	}

	std::ifstream file;
	if (findFolderTag(tagPath,tag,file,path))
	{
		int sniplength;

		file.seekg(0, std::ios::end);
		sniplength = file.tellg();
		file.seekg(0, std::ios::beg);
		snip = new char[sniplength + 1];
        file.read(snip, sniplength);
		snip[sniplength] = '\0';
        file.close();
	}

	return snip;
}


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
        bool tagFound = false;

        int posBeforeTag=0;
    
	    int posCurrent= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        int posSelectionStart= ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
        int posSelectionEnd= ::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0);

        if (posSelectionStart!=posSelectionEnd)
        {
            //::MessageBox(nppData._nppHandle, TEXT("selection"), TEXT("Trace"), MB_OK);
        } else
        {
            
			char *tag;
			int tagLength = getCurrentTag(curScintilla, posCurrent, &tag);
            if (tagLength != 0)
			{
                //::MessageBox(nppData._nppHandle, (LPCWSTR)tag, TEXT("Trace"), MB_OK);
                // Here the tag is got assuming the document is in ANSI, if the document is in UTF-8,
                // chinese character tag is not loaded
                if (::SendMessage(curScintilla,SCI_GETCODEPAGE,0,0)==65001)
                {
                    //::MessageBox(nppData._nppHandle, TEXT("65001"), TEXT("Trace"), MB_OK);
                    //convertEncoding(tag,CP_UTF8,CP_ACP);
                    WCHAR *w=new WCHAR[tagLength + 1];
                    MultiByteToWideChar(CP_UTF8, 0, tag, -1, w, tagLength); 
                    WideCharToMultiByte(CP_ACP, 0, w, -1, tag, tagLength, 0, 0); 
                    delete [] w;
                }


				char *expanded = findTag(tag);
				if (expanded)
                {
                    replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
					tagFound = true;
					delete [] expanded;
                } 
				else
                {
                    expanded = findTag(tag, TEXT("Global"));
                    if (expanded)
                    {
                        replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
						tagFound = true;
						delete [] expanded;
                    } 
                    
                }

				delete [] tag;
                // return to the original path 
               // ::SetCurrentDirectory(curPath);
            }


            // return to the original position 
			// Don't need to do that anymore - we don't change the position
            //::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
        }
        	  
        bool spotFound = hotSpotNavigation(curScintilla);

        if ((spotFound == false) && (tagFound == false)) 
		{
			restoreTab(curScintilla, posCurrent, posSelectionStart, posSelectionEnd);
		}

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
    ::SendMessage(curScintilla,SCI_SETSELECTION,posSelectionStart,posSelectionEnd);
    //::SendMessage(curScintilla,SCI_SETSELECTIONSTART,posSelectionStart,0);
    //::SendMessage(curScintilla,SCI_SETSELECTIONEND,posSelectionEnd,0);
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

        ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos+4,secondPos);
        //::SendMessage(curScintilla,SCI_SETSELECTIONSTART,firstPos+4,0);
		//::SendMessage(curScintilla,SCI_SETSELECTIONEND,secondPos,0);
        //int selectionLength;

        //selectionLength = ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, NULL);
        char *hotSpotText = new char[secondPos - (firstPos + 4) + 1];
        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(hotSpotText));
        //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)hotSpotText);
        //char hotSpotText[100];
        //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&hotSpotText);

        ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos+3);
        
        //selectionLength = ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, NULL);
        char *hotSpot = new char[secondPos+3 - firstPos + 1];
        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(hotSpot));
        //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)hotSpot);
        //char hotSpot[100];
        //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&hotSpot);


        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
        ::SendMessage(curScintilla,SCI_GOTOPOS,secondPos+3,0);
        
        int hotSpotFound=-1;
        int tempPos[100];

        int i=1;

        for (i=1;i<=90;i++)
        {
            tempPos[i]=0;
        
            ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
            hotSpotFound=::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)hotSpot);
            if (hotSpotFound>=0)
            {
                //::MessageBox(nppData._nppHandle, TEXT(">=0"), TEXT("Trace"), MB_OK);
                tempPos[i] = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
                ::SendMessage(curScintilla,SCI_GOTOPOS,tempPos[i]+1,0);
            } else
            {
                break;
                //::MessageBox(nppData._nppHandle, TEXT("<0"), TEXT("Trace"), MB_OK);
                //tempPos[i]=-1;
            }
        }

        delete [] hotSpot;
        delete [] hotSpotText;
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


bool replaceTag(HWND &curScintilla, char *expanded, int &posCurrent, int &posBeforeTag)
{
    int preserveSteps=1;
    //::MessageBox(nppData._nppHandle, TEXT("replace tag"), TEXT("Trace"), MB_OK); 
    //std::streamoff sniplength;
    
    if (preserveSteps==0) 
		::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

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
			int snipLength = strlen(expanded);
            WCHAR *w=new WCHAR[snipLength + 1];
            MultiByteToWideChar(CP_ACP, 0, expanded, -1, w, snipLength); // ANSI to UNICODE
            WideCharToMultiByte(CP_UTF8, 0, w, -1, expanded, snipLength, 0, 0); // UNICODE to UTF-8
            delete [] w;
        }
  
		::SendMessage(curScintilla, SCI_SETTARGETSTART, posBeforeTag, 0);
		::SendMessage(curScintilla, SCI_SETTARGETEND, posCurrent, 0);
        ::SendMessage(curScintilla, SCI_REPLACETARGET, strlen(expanded), reinterpret_cast<LPARAM>(expanded));
      
        ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
        ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"`[SnippetInserting]");
        int posEndOfInsertedText= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;
            
        ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
        ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
        ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"[>END<]");
        int posEndOfSnippet= ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        
        
        ::SendMessage(curScintilla,SCI_SETSELECTION,posEndOfSnippet,posEndOfInsertedText);
        //::SendMessage(curScintilla, SCI_SETSELECTIONSTART, posEndOfSnippet,(LPARAM)true);
        //::SendMessage(curScintilla, SCI_SETSELECTIONEND, posEndOfInsertedText,(LPARAM)true);

        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
		::SendMessage(curScintilla, SCI_GOTOPOS, posCurrent, 0);
        // This cause problem when we trigger a long snippet and then a short snippet
        // The problem is solved after using a better way to search for [>END<]
        if (preserveSteps==0) 
		{
			::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
		}
        return true;
}



int findFolderTag(TCHAR *tagPath, char *tag, std::ifstream &file,TCHAR *path)
{

    int folderFound=static_cast<int>(::SetCurrentDirectory(path));
	
	if (folderFound > 0)
		folderFound = static_cast<int>(::SetCurrentDirectory(tagPath));
    
	if (folderFound <= 0)
    {
        return 0;
    }
    else
    {
        file.open(tag, std::ios::binary);
        if (file.is_open())
        {
            return 1;
        } else
        {
            ::SetCurrentDirectory(path);
            return 0;
        }
    }
}
