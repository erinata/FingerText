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
///////////////////////////////////////////////////////
//
//This file is modified from the NppPluginTemplate provided by Don Ho. 
//The License of the template is shown below. 
//
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
#include "sqlite3.h"
#include "SnippetDock.h"
#include "VersionNumber.h"

#include <fstream>

//#include <string.h>

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

sqlite3 *g_db;
bool     g_dbOpen;

struct SnipIndex {
    char* triggerText;
    char* scope;
    char* content;
};

TCHAR g_iniPath[MAX_PATH];
TCHAR g_ftbPath[MAX_PATH];

SnipIndex* g_snippetCache;
int g_snippetCacheSize;

bool g_enable;
bool g_editorView;
//bool g_display;


//char* snippetEditTemplate = "------ FingerText Snippet Editor View ------\r\n";

// Config file content
int g_version;
int g_snippetListLength;
int g_snippetListOrderTagType;
int g_tabTagCompletion;
int g_liveHintUpdate;
int g_indentReference;
int g_chainLimit;
int g_preserveSteps;

bool g_newUpdate;

DockingDlg snippetDock;

#define TRIGGER_TAB_INDEX 0
#define SNIPPET_DOCK_INDEX 1
#define TOGGLE_ENABLE_INDEX 2
#define IMPORT_SNIPPETS_INDEX 3
#define EXPORT_SNIPPETS_INDEX 4
#define SEPARATOR_ONE_INDEX 5
#define TAG_COMPLETE_INDEX 6
#define SEPARATOR_TWO_INDEX 7
#define HELP_INDEX 8
#define ABOUT_INDEX 9
#define SEPARATOR_THREE_INDEX 10
#define TESTING_INDEX 11
//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
    snippetDock.init((HINSTANCE)hModule, NULL);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
    g_liveHintUpdate = 2;
    //writeConfig();
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
    
    setCommand(TRIGGER_TAB_INDEX, TEXT("Trigger FingerText"), fingerText, shKey, false);
    setCommand(SNIPPET_DOCK_INDEX, TEXT("Toggle On/off SnippetDock"), showSnippetDock, NULL, false);
    setCommand(TOGGLE_ENABLE_INDEX, TEXT("Toggle On/Off FingerText"), toggleDisable, NULL, false);
    setCommand(IMPORT_SNIPPETS_INDEX, TEXT("Import Snippets"), importSnippets, NULL, false);
    setCommand(EXPORT_SNIPPETS_INDEX, TEXT("Export Snippets"), exportSnippets, NULL, false);
    setCommand(SEPARATOR_ONE_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(TAG_COMPLETE_INDEX, TEXT("Tag Completion"), tagComplete, NULL, false);
    setCommand(SEPARATOR_TWO_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(HELP_INDEX, TEXT("Quick Guide"), showHelp, NULL, false);
    setCommand(ABOUT_INDEX, TEXT("About"), showAbout, NULL, false);
    setCommand(SEPARATOR_THREE_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(TESTING_INDEX, TEXT("Testing"), testing, NULL, false);

    setConfigAndDatabase();
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
    //delete [] snippetEditTemplate;
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

void toggleDisable()
{
    if (g_enable)
    {
        ::MessageBox(nppData._nppHandle, TEXT("FingerText is disabled"), TEXT("Trace"), MB_OK);
        g_enable = false;
    } else
    {
        ::MessageBox(nppData._nppHandle, TEXT("FingerText is enabled"), TEXT("Trace"), MB_OK);
        g_enable = true;
    }
    updateMode();
}

char *findTagSQLite(char *tag, int level, TCHAR* scope=TEXT(""), bool similar=false)
{
	char *expanded = NULL;
	sqlite3_stmt *stmt;

    // First create the SQLite SQL statement ("prepare" it for running)
    int sqlitePrepare;
    if (similar == false)
    {
        sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag", -1, &stmt, NULL);
    } else
    {
        sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag", -1, &stmt, NULL);
    }
    

	if (g_dbOpen && SQLITE_OK == sqlitePrepare)
	{
        char *tagType = NULL;
        if (level == 0)
        {
            convertToUTF8(scope, &tagType);
        } else
        {            
            TCHAR *fileType = NULL;
            fileType = new TCHAR[MAX_PATH];
            ::swprintf(fileType,TEXT("GLOBAL"));
            if (level == 1)
            {
	            ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
            } else if (level == 2)
            {
	            ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
            }
        
            convertToUTF8(fileType, &tagType);
            delete [] fileType;
        }
		
		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
		sqlite3_bind_text(stmt, 1, tagType, -1, SQLITE_STATIC);

        if (similar==1)
        {
            char similarTag[MAX_PATH]="";
            strcat(similarTag,"%");
            strcat(similarTag,tag);
            strcat(similarTag,"%");
            //::SendMessage(getCurrentScintilla(),SCI_INSERTTEXT,0,(LPARAM)similarTag);
            sqlite3_bind_text(stmt, 2, similarTag, -1, SQLITE_STATIC);
        } else
        {

		    sqlite3_bind_text(stmt, 2, tag, -1, SQLITE_STATIC);
        }

		// Run the query with sqlite3_step
		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
		{
			const char* expandedSQL = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
			expanded = new char[strlen(expandedSQL)*4 + 1];
			strcpy(expanded, expandedSQL);
		}
	}
    // Close the SQLite statement, as we don't need it anymore
	// This also has the effect of free'ing the result from sqlite3_column_text 
	// (i.e. in our case, expandedSQL)
	sqlite3_finalize(stmt);
	return expanded; //remember to delete the returned expanded after use.
}

void upgradeMessage()
{
    // This message is supressed for now and can be used later
    //if (g_newUpdate)
    //{
    //
    //    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
    //    ::SendMessage(getCurrentScintilla(), SCI_INSERTTEXT, 0, (LPARAM)WELCOME_TEXT);
    //}
}
//void insertSnippet()
//{
//    sqlite3_stmt *stmt;
//
//    HWND curScintilla = getCurrentScintilla();
//    int posCurrent = ::SendMessage(curScintilla, SCI_GETCURRENTPOS,0,0);
//
//    int index = snippetDock.getCount() - snippetDock.getSelection()-1;
//    
//    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType=? AND tag=?", -1, &stmt, NULL))
//	{
//		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
//		sqlite3_bind_text(stmt, 1, g_snippetCache[index].scope , -1, SQLITE_STATIC);
//		sqlite3_bind_text(stmt, 2, g_snippetCache[index].triggerText, -1, SQLITE_STATIC);
//
//		// Run the query with sqlite3_step
//		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
//		{
//			const char* snippetText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
//            char * tempSnippetText;
//            tempSnippetText = new char [strlen(snippetText)+1];
//            strcpy(tempSnippetText, snippetText);
//            replaceTag(curScintilla, tempSnippetText, posCurrent, posCurrent);
//            
//            delete [] tempSnippetText;
//            //::SendMessage(curScintilla, SCI_INSERTTEXT, posCurrent, (LPARAM)snippetText);
//		}	
//	}
//	sqlite3_finalize(stmt);
//    ::SendMessage(curScintilla,SCI_GRABFOCUS,0,0); 
//
//}

void createSnippet()
{
    //::MessageBox(nppData._nppHandle, TEXT("CREATE~!"), TEXT("Trace"), MB_OK);
    //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
    HWND curScintilla = getCurrentScintilla();

    if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
    {
        ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_ftbPath);
    } 
    promptSaveSnippet(TEXT("Do you wish to save the current snippet before creating a new one?"));
    
    ::SendMessage(curScintilla,SCI_CLEARALL,0,0);

    ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)SNIPPET_EDIT_TEMPLATE);
    ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)"newtriggertext\r\nGLOBAL\r\nThis is a new snippet.\r\nThis new snippet is awesome.\r\nNew snippet is here.\r\n[>END<]");
    
    //::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
    ::SendMessage(curScintilla,SCI_EMPTYUNDOBUFFER,0,0);

    g_editorView = true;
    updateDockItems(false,false);
    updateMode();
    refreshAnnotation();
}


void editSnippet()
{
    HWND curScintilla = getCurrentScintilla();
    int index = snippetDock.getCount() - snippetDock.getSelection()-1;
    char *tempTriggerText;
    char *tempScope;
    //char * tempSnippetText;
    tempTriggerText = new char [strlen(g_snippetCache[index].triggerText)];
    tempScope = new char [strlen(g_snippetCache[index].scope)];

    //strcpy(tempTriggerText, g_snippetCache[index].triggerText);
    //strcpy(tempScope, g_snippetCache[index].scope);
    sprintf(tempTriggerText, g_snippetCache[index].triggerText);
    sprintf(tempScope, g_snippetCache[index].scope);
    
    sqlite3_stmt *stmt;

    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType = ? AND tag = ?", -1, &stmt, NULL))
	{
		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
		sqlite3_bind_text(stmt, 1, tempScope , -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, tempTriggerText, -1, SQLITE_STATIC);
		// Run the query with sqlite3_step
		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
		{
			const char* snippetText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column

            // After loading the content, switch to the editor buffer and promput for saving if needed
            if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
            {
                ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_ftbPath);
            }

            promptSaveSnippet(TEXT("Do you wish to save the current snippet before editing anotoher one?"));
            ::SendMessage(curScintilla,SCI_CLEARALL,0,0);
               
            //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)SNIPPET_EDIT_TEMPLATE);
            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)tempTriggerText);
            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)"\r\n");
            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)tempScope);
            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)"\r\n");

            ::SendMessage(curScintilla, SCI_INSERTTEXT, ::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)snippetText);
            g_editorView = true;
            refreshAnnotation();
		}		
	}
	sqlite3_finalize(stmt);

    ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
    ::SendMessage(curScintilla,SCI_EMPTYUNDOBUFFER,0,0);

    //delete [] tempTriggerText; // deleting them cause error
    //delete [] tempScope;
}

void deleteSnippet()
{
    HWND curScintilla = getCurrentScintilla();
    int index = snippetDock.getCount() - snippetDock.getSelection()-1;

    sqlite3_stmt *stmt;
    
    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
    {
        sqlite3_bind_text(stmt, 1, g_snippetCache[index].scope, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, g_snippetCache[index].triggerText, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    
    updateDockItems(false,false);    
}


bool getLineChecked(char **buffer, HWND &curScintilla, int lineNumber, TCHAR* errorText)
{
    bool problemSnippet;

    ::SendMessage(curScintilla,SCI_GOTOLINE,lineNumber,0);
    int tagPosStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    int tagPosEnd;
    if (lineNumber == 3)
    {
        tagPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
    } else
    {

        int tagPosLineEnd = ::SendMessage(curScintilla,SCI_GETLINEENDPOSITION,lineNumber,0);
        ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" ");
        tagPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        if (tagPosEnd>tagPosLineEnd)
        {
            tagPosEnd=tagPosLineEnd;
        }
    }

    if (tagPosEnd-tagPosStart<=0)
    {
        ::SendMessage(curScintilla,SCI_GOTOLINE,lineNumber,0);
        ::MessageBox(nppData._nppHandle, errorText, TEXT("FingerText"), MB_OK);
        problemSnippet = true;
    }

    if (lineNumber == 3)
    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,tagPosStart,0);

        ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
	    int spot=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"[>END<]");
        if (spot<0)
        {
            ::MessageBox(nppData._nppHandle, TEXT("You should put an \"[>END<]\" (without quotes) at the end of your snippet content."), TEXT("FingerText"), MB_OK);
            problemSnippet = true;
        }
    }

    ::SendMessage(curScintilla,SCI_SETSELECTION,tagPosStart,tagPosEnd);
    *buffer = new char[tagPosEnd-tagPosStart + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*buffer));

    return problemSnippet;
}


//TODO: saveSnippet() and importSnippet() need refactoring sooooooo badly..................
void saveSnippet()
{
    HWND curScintilla = getCurrentScintilla();

    int docLength = ::SendMessage(curScintilla, SCI_GETLENGTH,0,0);
    // insert a space at the end of the doc so the ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" "); will not get into error
    ::SendMessage(curScintilla, SCI_INSERTTEXT, docLength, (LPARAM)" ");
    
    bool problemSnippet = false;

    char* tagText;
    char* tagTypeText;
    char* snippetText;

    if (getLineChecked(&tagText,curScintilla,1,TEXT("TriggerText cannot be blank."))==true) problemSnippet = true;
    if (getLineChecked(&tagTypeText,curScintilla,2,TEXT("Scope cannot be blank."))==true) problemSnippet = true;
    if (getLineChecked(&snippetText,curScintilla,3,TEXT("Snippet Content cannot be blank."))==true) problemSnippet = true;
    
    ::SendMessage(curScintilla,SCI_SETSELECTION,docLength,docLength+1); //Take away the extra space added
    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");

    if (!problemSnippet)
    {
        // checking for existing snippet 
        sqlite3_stmt *stmt;

        if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
        {
            sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
		    sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
            if(SQLITE_ROW == sqlite3_step(stmt))
            {
                sqlite3_finalize(stmt);
                int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("Snippet exists, overwrite?"), TEXT("FingerText"), MB_YESNO);
                if (messageReturn==IDNO)
                {
                    delete [] tagText;
                    delete [] tagTypeText;
                    delete [] snippetText;
                    // not overwrite
                    ::MessageBox(nppData._nppHandle, TEXT("The Snippet is not saved."), TEXT("FingerText"), MB_OK);
                    //::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
                    //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)" ");
                    ::SendMessage(curScintilla, SCI_SETSELECTION, 0, 1);
                    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"-");
                    ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
                
                    return;

                } else
                {
                    // delete existing entry
                    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                    {
                        sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
		                sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                        sqlite3_step(stmt);
                    
                    } else
                    {
                        ::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT("FingerText"), MB_OK);
                    }
                
                }

            } else
            {
                sqlite3_finalize(stmt);
            }
        }
    
        if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
	    {
		    // Then bind the two ? parameters in the SQLite SQL to the real parameter values
		    sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
		    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
    
		    // Run the query with sqlite3_step
		    sqlite3_step(stmt); // SQLITE_ROW 100 sqlite3_step() has another row ready
            ::MessageBox(nppData._nppHandle, TEXT("The Snippet is saved."), TEXT("FingerText"), MB_OK);
	    }
        sqlite3_finalize(stmt);
        ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
    }
    delete [] tagText;
    delete [] tagTypeText;
    delete [] snippetText;
    
    updateDockItems(false,false);
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
    ::SendMessage(curScintilla,SCI_TAB,0,0);	
}

void chainSnippet(HWND &curScintilla, int startingPos)
{
    char chainTagSign[] = "$[![(chain)";
    int chainTagSignLength = strlen(chainTagSign);
    char* chainHotSpotText;
    char* chainHotSpot;
    int chainSpot = -1;

    int chainLimitCounter = 0;
    do 
    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos,0);
        ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        chainSpot=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)chainTagSign);
        if (chainSpot>=0)
	    {
            int chainFirstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
            int chainSecondPos = grabHotSpotContent(curScintilla, &chainHotSpotText, &chainHotSpot, chainFirstPos,chainTagSignLength);
            
            int triggerPos = strlen(chainHotSpotText)+chainFirstPos;
            ::SendMessage(curScintilla,SCI_GOTOPOS,triggerPos,0);
            triggerTag(triggerPos,strlen(chainHotSpotText));
        }
        chainLimitCounter++;
    } while ((chainSpot>=0) && (chainLimitCounter<g_chainLimit));

}

void keyWordSpot(HWND &curScintilla, int startingPos)
{
    char keyTagSign[] = "$[![(key)";
    int keyTagSignLength = strlen(keyTagSign);
    char* keyHotSpotText;
    char* keyHotSpot;
    int keySpot = -1;

    //int keyLimitCounter = 0;
    do 
    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos,0);
        ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        keySpot=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)keyTagSign);
        if (keySpot>=0)
	    {
            int keyFirstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
            int keySecondPos = grabHotSpotContent(curScintilla, &keyHotSpotText, &keyHotSpot, keyFirstPos,keyTagSignLength);

            int triggerPos = strlen(keyHotSpotText)+keyFirstPos;
            //::SendMessage(curScintilla,SCI_GOTOPOS,triggerPos,0);
            ::SendMessage(curScintilla,SCI_SETSEL,keyFirstPos,triggerPos);
            //TODO: probably should use enum type...........

            if (strcmp(keyHotSpotText,"PASTE")==0)
            {
                ::SendMessage(curScintilla,SCI_PASTE,0,0);
	            
            } else if (strcmp(keyHotSpotText,"DATESHORT")==0)
            {
                insertDateTime(true,DATE_SHORTDATE,curScintilla);
	            
            } else if (strcmp(keyHotSpotText,"DATELONG")==0)
            {
                insertDateTime(true,DATE_LONGDATE,curScintilla);
                
            } else if (strcmp(keyHotSpotText,"TIMESHORT")==0)
            {
                insertDateTime(false,TIME_NOSECONDS,curScintilla);
                
            } else if (strcmp(keyHotSpotText,"TIMELONG")==0)
            {
                insertDateTime(false,0,curScintilla);
            } else if (strcmp(keyHotSpotText,"FILENAME")==0)
            {
                insertCurrentPath(NPPM_GETNAMEPART,curScintilla);
                
            } else if (strcmp(keyHotSpotText,"EXTNAME")==0)
            {
                insertCurrentPath(NPPM_GETEXTPART,curScintilla);
                
            } else if (strcmp(keyHotSpotText,"DIRECTORY")==0)
            {
                insertCurrentPath(NPPM_GETCURRENTDIRECTORY,curScintilla);
                
            }
            
            //triggerTag(triggerPos);
        }
        //keyLimitCounter++;
    } while (keySpot>=0);
    //while ((keySpot>=0) && (keyLimitCounter<g_keyLimit));

}

void runCommand()
{
}

void insertCurrentPath(int msg, HWND &curScintilla)
{

	TCHAR path[MAX_PATH];
	::SendMessage(nppData._nppHandle, msg, 0, (LPARAM)path);

	char pathText[MAX_PATH];
	WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, path, -1, pathText, MAX_PATH, NULL, NULL);
	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)pathText);
}

void insertDateTime(bool date,int type, HWND &curScintilla)
{
    TCHAR time[128];
    SYSTEMTIME formatTime;
	::GetLocalTime(&formatTime);
    if (date)
    {
        ::GetDateFormat(LOCALE_USER_DEFAULT, type, &formatTime, NULL, time, 128);
    } else
    {
        ::GetTimeFormat(LOCALE_USER_DEFAULT, type, &formatTime, NULL, time, 128);
    }
	
	char timeText[MAX_PATH];
	WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, time, -1, timeText, MAX_PATH, NULL, NULL);
	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)timeText);

}


bool hotSpotNavigation(HWND &curScintilla)
{
    //bool preserveSteps=false;

    // This is the part doing Hotspots tab navigation
    //::SendMessage(curScintilla,SCI_GOTOPOS,startingPos,0);

    char tagSign[] = "$[![";
    int tagSignLength = strlen(tagSign);

    char *hotSpotText;
    char *hotSpot;

    ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
    int tagSpot=::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)tagSign);
        
	if (tagSpot>=0)
	{
        if (g_preserveSteps==1) ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
        int firstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        int secondPos = grabHotSpotContent(curScintilla, &hotSpotText, &hotSpot, firstPos,tagSignLength);
                
        int hotSpotFound=-1;
        int tempPos[100];

        int i=1;

        for (i=1;i<=98;i++)
        {
            tempPos[i]=0;
        
            ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
            hotSpotFound=::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)hotSpot);
            if (hotSpotFound>=0)
            {
                //::MessageBox(nppData._nppHandle, TEXT(">=0"), TEXT("Trace"), MB_OK);
                
                tempPos[i] = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
                ::SendMessage(curScintilla,SCI_GOTOPOS,tempPos[i],0);
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
        
        ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos-tagSignLength);
        for (int j=1;j<i;j++)
        {
            if (tempPos[j]!=-1)
            {
                ::SendMessage(curScintilla,SCI_ADDSELECTION,tempPos[j],tempPos[j]+(secondPos-tagSignLength-firstPos));
            }
        }
        ::SendMessage(curScintilla,SCI_SETMAINSELECTION,0,0);
        ::SendMessage(curScintilla,SCI_LINESCROLL,0,0);
        

        if (g_preserveSteps==1) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
        return true;
	} else
    {
        //delete [] hotSpot;  // Don't try to delete if it has not been initialized
        //delete [] hotSpotText;
        return false;
    }

    //::MessageBox(nppData._nppHandle, TEXT("<0"), TEXT("Trace"), MB_OK);
    return false;
}


int grabHotSpotContent(HWND &curScintilla, char **hotSpotText,char **hotSpot, int firstPos, int signLength)
{

    int posLine = ::SendMessage(curScintilla,SCI_LINEFROMPOSITION,0,0);
        
	::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
	::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"]!]");
	int secondPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos+signLength,secondPos);

    *hotSpotText = new char[secondPos - (firstPos + signLength) + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpotText));

    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos+3);
    
    *hotSpot = new char[secondPos+3 - firstPos + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpot));

    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)*hotSpotText);
    ::SendMessage(curScintilla,SCI_GOTOPOS,secondPos+3,0);
    
    return secondPos;  
}

void showPreview()
{
    sqlite3_stmt *stmt;

    HWND curScintilla = getCurrentScintilla();
    int posCurrent = ::SendMessage(curScintilla, SCI_GETCURRENTPOS,0,0);

    int index = snippetDock.getCount() - snippetDock.getSelection()-1;
    
    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
	{
		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
		sqlite3_bind_text(stmt, 1, g_snippetCache[index].scope , -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, g_snippetCache[index].triggerText, -1, SQLITE_STATIC);

		// Run the query with sqlite3_step
		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
		{
			const char* snippetText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
            //size_t origsize = strlen(snippetText) + 1; 
            size_t convertedChars = 0; 
            wchar_t previewText[270]; 
            mbstowcs_s(&convertedChars, previewText, 190, snippetText, _TRUNCATE);
            
            if (convertedChars>=184)
            {
                const TCHAR etcText[] = TEXT(" . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .");
                ::_tcscat(previewText,etcText);
            }
            
            snippetDock.setDlgText(ID_SNIPSHOW_EDIT,previewText);

            //wchar_t countText[10];
            //::_itow_s(convertedChars, countText, 10, 10); 
            //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);
            
            //replaceTag(curScintilla, tempSnippetText, posCurrent, posCurrent);
            
            //::SendMessage(curScintilla, SCI_INSERTTEXT, posCurrent, (LPARAM)snippetText);
		}	
	}
	sqlite3_finalize(stmt);
    //::SendMessage(curScintilla,SCI_GRABFOCUS,0,0); 
}

bool replaceTag(HWND &curScintilla, char *expanded, int &posCurrent, int &posBeforeTag)
{
    
    //TODO: can use ::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line-1, 0); to make sure that caret is visible after long snippet substitution.
    //bool preserveSteps=false;

    //::MessageBox(nppData._nppHandle, TEXT("replace tag"), TEXT("Trace"), MB_OK); 
    //std::streamoff sniplength;
    int lineCurrent = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, posCurrent, 0);
    int initialIndent = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, lineCurrent, 0);

    /////////////////
    //::SendMessage(curScintilla,SCI_GOTOLINE,lineCurrent,0);
    //int posLineStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
    //::SendMessage(curScintilla,SCI_SETSELECTION,posLineStart,posLineStart+initialIndent);
    //
    //char* indentText = new char[initialIndent + 1];
    //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(indentText));
    /////////////////////

    //if (preserveSteps==false) 
    //{
    //    ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
    //}

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
        // This is not needed for sqlite system as sqlite database is in utf-8
        //if (::SendMessage(curScintilla,SCI_GETCODEPAGE,0,0)==65001)
        //{
        //    //::MessageBox(nppData._nppHandle, TEXT("65001"), TEXT("Trace"), MB_OK);
		//	int snipLength = strlen(expanded);
        //    WCHAR *w=new WCHAR[snipLength*4+1];
        //    MultiByteToWideChar(CP_ACP, 0, expanded, -1, w, snipLength*4+1); // ANSI to UNICODE
        //    WideCharToMultiByte(CP_UTF8, 0, w, -1, expanded, snipLength*4+1, 0, 0); // UNICODE to UTF-8
        //    delete [] w;
        //}
  
	::SendMessage(curScintilla, SCI_SETTARGETSTART, posBeforeTag, 0);
	::SendMessage(curScintilla, SCI_SETTARGETEND, posCurrent, 0);
    ::SendMessage(curScintilla, SCI_REPLACETARGET, strlen(expanded), reinterpret_cast<LPARAM>(expanded));

    ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
    ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"`[SnippetInserting]");
    int posEndOfInsertedText = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;

    // adjust indentation according to initial indentation
    if (g_indentReference==1)
    {
        int lineInsertedSnippet = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, posEndOfInsertedText, 0);

        int lineIndent=0;
        for (int i=lineCurrent+1;i<=lineInsertedSnippet;i++)
        {
            /////////////
            lineIndent = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, i, 0);
        
            //::SendMessage(curScintilla, SCI_GOTOLINE, i, 0);
            //::SendMessage(curScintilla, SCI_REPLACESEL, 0 , (LPARAM)indentText);
            ///////////
            ::SendMessage(curScintilla, SCI_SETLINEINDENTATION, i, initialIndent+lineIndent);
        }

        //delete [] indentText;
    }
    
    ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
    ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"`[SnippetInserting]");
    posEndOfInsertedText = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;
                    
    ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
    ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
    ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)"[>END<]");
    //::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
    int posEndOfSnippet = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        
    ::SendMessage(curScintilla,SCI_SETSELECTION,posEndOfSnippet,posEndOfInsertedText);

    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
	::SendMessage(curScintilla, SCI_GOTOPOS, posCurrent, 0);
    // This cause problem when we trigger a long snippet and then a short snippet
    // The problem is solved after using a better way to search for [>END<]

    //if (preserveSteps==false) 
	//{
	//	::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	//}

    
    return true;
}


void pluginShutdown()  // function is triggered when NPPN_SHUTDOWN fires.
{
    delete [] g_snippetCache;
    if (g_dbOpen)
    {
        sqlite3_close(g_db);  // This close the database when the plugin shutdown.
        g_dbOpen = false;
    }
}

void setConfigAndDatabase()
{
    g_enable = true;
    //g_display=false;
    updateMode();
    
    TCHAR path[MAX_PATH];
    char *cpath;
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(path));
    int multibyteLength = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, 0, 0);
    cpath = new char[multibyteLength + 50];
    WideCharToMultiByte(CP_UTF8, 0, path, -1, cpath, multibyteLength, 0, 0);
    strcat(cpath, "\\FingerText.db3");
    
    if (PathFileExists(path) == FALSE) 
    {
		::CreateDirectory(path, NULL);
	}

    int rc = sqlite3_open(cpath, &g_db);
    if (rc)
    {
        g_dbOpen = false;
        MessageBox(nppData._nppHandle, TEXT("Cannot find or open FingerText.db3 in config folder"), TEXT("FingerText plugin"), MB_ICONERROR);
    }
    else
    {
        g_dbOpen = true;
    }

    sqlite3_stmt *stmt;

    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "CREATE TABLE snippets (tag TEXT, tagType TEXT, snippet TEXT)", -1, &stmt, NULL))
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Loading config file

    ::_tcscpy(g_iniPath,path);
    ::_tcscpy(g_ftbPath,path);
    ::_tcscat(g_iniPath,TEXT("\\FingerText.ini"));
    ::_tcscat(g_ftbPath,TEXT("\\SnippetEditor.ftb"));
    //::MessageBox(nppData._nppHandle, path, TEXT("Trace"), MB_OK);

    setupConfigFile();

    if (PathFileExists(g_ftbPath) == FALSE)
	{
        ::WritePrivateProfileString(TEXT("Dummy"), TEXT("Dummy"), TEXT("Dummy"), g_ftbPath);
        //::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILEAS, 0, (LPARAM)ftbPath);
    }
}

void setupConfigFile()
{

    g_version = ::GetPrivateProfileInt(TEXT("FingerText"), TEXT("version"), 0, g_iniPath);
    g_snippetListLength = GetPrivateProfileInt(TEXT("FingerText"), TEXT("snippet_list_length"), 0, g_iniPath);
    g_snippetListOrderTagType = GetPrivateProfileInt(TEXT("FingerText"), TEXT("snippet_list_order_tagtype"), 0, g_iniPath);
    g_tabTagCompletion = GetPrivateProfileInt(TEXT("FingerText"), TEXT("tab_tag_completion"), 0, g_iniPath);
    g_liveHintUpdate = GetPrivateProfileInt(TEXT("FingerText"), TEXT("live_hint_update"), 0, g_iniPath);
    g_indentReference = GetPrivateProfileInt(TEXT("FingerText"), TEXT("indent_reference"), 0, g_iniPath);
    g_chainLimit = GetPrivateProfileInt(TEXT("FingerText"), TEXT("chain_limit"), 0, g_iniPath);
    g_preserveSteps = GetPrivateProfileInt(TEXT("FingerText"), TEXT("preserve_steps"), 0, g_iniPath);

    if (g_version != VERSION_LINEAR)
    {
        g_version = VERSION_LINEAR;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("version"), VERSION_LINEAR_TEXT, g_iniPath);
        g_newUpdate = true;
        
        //TCHAR welcomeMessage[MAX_PATH] = TEXT("");
        //::_tcscat(welcomeMessage,TEXT("Thanks for Upgrading to "));
        //::_tcscat(welcomeMessage,VERSION_TEXT_LONG_WIDE);
        //
        //::MessageBox(nppData._nppHandle, welcomeMessage, TEXT("FingerText"), MB_OK);

    } else
    {
        g_newUpdate = false;
    }

    if (g_snippetListLength==NULL)
    {
        g_snippetListLength = 100;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("snippet_list_length"), TEXT("100"), g_iniPath);
    }
    if (g_snippetListOrderTagType==NULL)
    {
        g_snippetListOrderTagType = 1;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("snippet_list_order_tagtype"), TEXT("1"), g_iniPath);
    }
    if (g_tabTagCompletion==NULL)
    {
        g_tabTagCompletion = 2;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("tab_tag_completion"), TEXT("2"), g_iniPath);
    } 
    if (g_liveHintUpdate==NULL)
    {
        g_liveHintUpdate = 1;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("live_hint_update"), TEXT("1"), g_iniPath);
    } 
    if (g_indentReference==NULL)
    {
        g_indentReference = 1;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("indent_reference"), TEXT("1"), g_iniPath);
    } 
    if (g_chainLimit==NULL)
    {
        g_chainLimit = 20;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("chain_limit"), TEXT("20"), g_iniPath);
    } 
    if (g_preserveSteps==NULL)
    {
        g_preserveSteps = 1;
        ::WritePrivateProfileString(TEXT("FingerText"), TEXT("preserve_steps"), TEXT("1"), g_iniPath);
    } 
}


int getCurrentTag(HWND curScintilla, int posCurrent, char **buffer, int triggerLength)
{
	int length = 0;

    //TODO: some better way to get the tag than WORDSTARTPOSITION
    int posBeforeTag;
    if (triggerLength<=0)
    {   
	    posBeforeTag = static_cast<int>(::SendMessage(curScintilla,	SCI_WORDSTARTPOSITION, posCurrent, 1));
    } else
    {
        posBeforeTag = posCurrent - triggerLength;
    }
                
    if (posCurrent - posBeforeTag < 100) // Max tag length 100
    {
        *buffer = new char[(posCurrent - posBeforeTag) + 1];
		Sci_TextRange range;
		range.chrg.cpMin = posBeforeTag;
		range.chrg.cpMax = posCurrent;
		range.lpstrText = *buffer;

	    ::SendMessage(curScintilla, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));
		length = (posCurrent - posBeforeTag);
	}
	return length;
}

void convertToUTF8(TCHAR *orig, char **utf8)
{
	int multibyteLength = WideCharToMultiByte(CP_UTF8, 0, orig, -1, NULL, 0, 0, 0);
	*utf8 = new char[multibyteLength + 1];
	WideCharToMultiByte(CP_UTF8, 0, orig, -1, *utf8, multibyteLength, 0, 0);
}

void showSnippetDock()
{
	snippetDock.setParent(nppData._nppHandle);
	tTbData	data = {0};

	if (!snippetDock.isCreated())
	{
		snippetDock.create(&data);

		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT;

		data.pszModuleName = snippetDock.getPluginFileName();

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		data.dlgID = SNIPPET_DOCK_INDEX;
		::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
        snippetDock.display();
	} else
    {
        snippetDock.display(!snippetDock.isVisible());
    }

   
}


void snippetHintUpdate()
{     
    if ((!g_editorView) && (g_liveHintUpdate==1))
    {
        HWND curScintilla = getCurrentScintilla();
        if (::SendMessage(curScintilla,SCI_GETMODIFY,0,0)!=0)
        {
            int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
            char *partialTag;
	        int tagLength = getCurrentTag(curScintilla, posCurrent, &partialTag);
            //
            //wchar_t countText[10];
            //::_itow_s(tagLength, countText, 10, 10); 
            //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);

            if ((tagLength>0) && (tagLength<30))
            {
                char similarTag[MAX_PATH] = "%";
                strcat(similarTag,partialTag);
                strcat(similarTag,"%");
        
                updateDockItems(false,false,similarTag);
            } else
            {
                updateDockItems(false,false);
            }
            delete [] partialTag;   

        }
    }
}



void updateDockItems(bool withContent, bool withAll, char* tag)
{
    int scopeLength=0;
    int triggerLength=0;
    int contentLength=0;
    int tempScopeLength=0;
    int tempTriggerLength=0;
    int tempContentLength=0;

    //g_snippetCacheSize=snippetDock.getLength();
    g_snippetCacheSize=g_snippetListLength;
    g_snippetCache = new SnipIndex [g_snippetCacheSize];
    clearCache();

    snippetDock.clearDock();
    sqlite3_stmt *stmt;
    
    if (g_editorView) withAll = true;

    // TODO: Use strcat instead of just nested if 
    int sqlitePrepare;
    if (g_snippetListOrderTagType==1)
    {
        if (withAll)
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets ORDER BY tagType,tag DESC LIMIT ? ", -1, &stmt, NULL);
        } else 
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets WHERE (tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ?) AND tag LIKE ? ORDER BY tagType,tag DESC LIMIT ? ", -1, &stmt, NULL);
        }

    } else
    {
        if (withAll)
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets ORDER BY tag,tagType DESC LIMIT ? ", -1, &stmt, NULL);
        } else 
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets WHERE (tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ?) AND tag LIKE ? ORDER BY tag DESC,tagType LIMIT ? ", -1, &stmt, NULL);
        }
    }
    
    
	if (g_dbOpen && SQLITE_OK == sqlitePrepare)
	{
        char *tagType1 = NULL;
        TCHAR *fileType1 = NULL;
		fileType1 = new TCHAR[MAX_PATH];
        char *tagType2 = NULL;
        TCHAR *fileType2 = NULL;
		fileType2 = new TCHAR[MAX_PATH];

        if (withAll)
        {
            char snippetCacheSizeText[10];
            ::_itoa(g_snippetCacheSize, snippetCacheSizeText, 10); 

            sqlite3_bind_text(stmt, 1, snippetCacheSizeText, -1, SQLITE_STATIC);

        } else
        {
            ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType1);
            convertToUTF8(fileType1, &tagType1);
            sqlite3_bind_text(stmt, 1, tagType1, -1, SQLITE_STATIC);
        		
            ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)fileType2);
            convertToUTF8(fileType2, &tagType2);
            sqlite3_bind_text(stmt, 2, tagType2, -1, SQLITE_STATIC);
        
            sqlite3_bind_text(stmt, 3, "GLOBAL", -1, SQLITE_STATIC);

            sqlite3_bind_text(stmt, 4, tag, -1, SQLITE_STATIC);

            //int cols = sqlite3_column_count(stmt);
            //tagType = itoa(snippetCacheSize,tagType,10);
            char snippetCacheSizeText[10];
            ::_itoa(g_snippetCacheSize, snippetCacheSizeText, 10); 

            sqlite3_bind_text(stmt, 5, snippetCacheSizeText, -1, SQLITE_STATIC);
        }
    
        int row = 0;

        while(true)
        {
            if(SQLITE_ROW == sqlite3_step(stmt))
            {
                const char* tempScope = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                tempScopeLength = strlen(tempScope)*4 + 1;
                if (tempScopeLength> scopeLength)
                {
                    scopeLength = tempScopeLength;
                }
                g_snippetCache[row].scope = new char[strlen(tempScope)*4 + 1];
                strcpy(g_snippetCache[row].scope, tempScope);

                const char* tempTrigger = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                tempTriggerLength = strlen(tempTrigger)*4 + 1;
                if (tempTriggerLength> triggerLength)
                {
                    triggerLength = tempTriggerLength;
                }
                g_snippetCache[row].triggerText = new char[strlen(tempTrigger)*4 + 1];
                strcpy(g_snippetCache[row].triggerText, tempTrigger);

                if (withContent)
                {
                    const char* tempContent = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                    tempContentLength = strlen(tempContent)*4 + 1;
                    if (tempContentLength> contentLength)
                    {
                        contentLength = tempContentLength;
                    }
                    g_snippetCache[row].content = new char[strlen(tempContent)*4 + 1];
                    strcpy(g_snippetCache[row].content, tempContent);
                }
                row++;

            }
            else
            {
                break;  
            }
        }

        
        delete [] tagType1;
        delete [] fileType1;

        delete [] tagType2;
        delete [] fileType2;

    }



    for (int j=0;j<g_snippetCacheSize;j++)
    {
        if (g_snippetCache[j].scope !=NULL)
        {
            char newText[200]=""; // TODO: this const length array problem has to be solve when we have scripting hotspot as the hotspot can get super long.
        
            strcat(newText,"<");
            strcat(newText,g_snippetCache[j].scope);
            strcat(newText,">   ");
            strcat(newText,g_snippetCache[j].triggerText);

            size_t origsize = strlen(newText) + 1;
            const size_t newsize = 300;
            size_t convertedChars = 0;
            wchar_t convertedTagText[newsize];
            mbstowcs_s(&convertedChars, convertedTagText, origsize, newText, _TRUNCATE);

            snippetDock.addDockItem(convertedTagText);
        }
    }

    HWND curScintilla = getCurrentScintilla();
    ::SendMessage(curScintilla,SCI_GRABFOCUS,0,0); 
    
    sqlite3_finalize(stmt);
}

void clearCache()
{
    for (int i=0;i<g_snippetCacheSize;i++)
    {
        g_snippetCache[i].triggerText=NULL;
        g_snippetCache[i].scope=NULL;
        g_snippetCache[i].content=NULL;
    }
}


void exportSnippets()
{
    if (g_liveHintUpdate == 1)  // Temporary turn off live update as it disturb exporting
    {
        g_liveHintUpdate = 2;
    } else 
    {
        g_liveHintUpdate = 3;
    }
    HWND curScintilla = getCurrentScintilla();

    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("FingerText Datafiles (*.ftd)\0*.ftd\0");
    ofn.lpstrFile = (LPWSTR)fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("");
    
    if (::GetSaveFileName(&ofn))
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
        int importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
        ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);

        updateDockItems(true,true,"%");

        int exportCount = 0;
        for (int j=0;j<g_snippetCacheSize;j++)
        {
            if (g_snippetCache[j].scope !=NULL)
            {
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].triggerText);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n");
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].scope);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n");
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].content);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"!$[FingerTextData FingerTextData]@#\r\n");
                exportCount++;
            }
        }
        ::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILEAS, 0, (LPARAM)fileName);

        wchar_t exportCountText[35] = TEXT("");

        if (exportCount>1)
        {
            ::_itow_s(exportCount, exportCountText, 10, 10);
            wcscat(exportCountText,TEXT(" snippets are exported."));
        } else if (exportCount==1)
        {
            wcscat(exportCountText,TEXT("1 snippet is exported."));
        } else
        {
            wcscat(exportCountText,TEXT("Snippet export is aborted."));
          
        }
        
        ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
        ::MessageBox(nppData._nppHandle, exportCountText, TEXT("FingerText"), MB_OK);
    }

    if (g_liveHintUpdate == 3)
    {
        g_liveHintUpdate = 2;
    } else 
    {
        g_liveHintUpdate = 1;
    }
    
}


//TODO: importsnippet and savesnippets need refactoring sooooo badly
void importSnippets()
{
    if (g_liveHintUpdate == 1)
    {
        g_liveHintUpdate = 2;
    } else 
    {
        g_liveHintUpdate = 3;
    }
    
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("FingerText Datafiles (*.ftd)\0*.ftd\0");
    ofn.lpstrFile = (LPWSTR)fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("");
    
    if (::GetOpenFileName(&ofn))
    {
        //::MessageBox(nppData._nppHandle, (LPCWSTR)fileName, TEXT("Trace"), MB_OK);
        std::ifstream file;

        file.open((LPCWSTR)fileName);   // This part may cause problem in chinese file names

        file.seekg(0, std::ios::end);
        int fileLength = file.tellg();
        file.seekg(0, std::ios::beg);

        //wchar_t countText[10];
        //::_itow_s(fileLength, countText, 10, 10); 
        //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);
        
        char* fileText = new char[fileLength+1];

        if (file.is_open())
        {
            file.read(fileText,fileLength);
            file.close();
        
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            int importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
            ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);

            HWND curScintilla = getCurrentScintilla();
            //::SendMessage(curScintilla, SCI_SETCODEPAGE,65001,0);
            ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)fileText);
            ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
            int importCount=0;
            int next=0;
            do
            {
                //import snippet do not have the problem of " " in save snippet because of the space in  "!$[FingerTextData FingerTextData]@#"
                
                char* tagText; 
                char* tagTypeText;
                
                getLineChecked(&tagText,curScintilla,0,TEXT("Error: Blank TriggerText. The ftd file may be corrupted."));
                getLineChecked(&tagTypeText,curScintilla,1,TEXT("Error: Blank Scope. The ftd file may be corrupted."));
                
                // Getting text after the 3rd line until the tag !$[FingerTextData FingerTextData]@#
                ::SendMessage(curScintilla,SCI_GOTOLINE,2,0);
                int snippetPosStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                //int snippetPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
                ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
                ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"!$[FingerTextData FingerTextData]@#");
                int snippetPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                ::SendMessage(curScintilla,SCI_SETSELECTION,snippetPosStart,snippetPosEnd);
            
                char* snippetText = new char[snippetPosEnd-snippetPosStart + 1];
                ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(snippetText));
            
                ::SendMessage(curScintilla,SCI_SETSELECTION,0,snippetPosEnd);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
                //TODO :need rewrite to more efficient code
                ::SendMessage(curScintilla,SCI_GOTOLINE,1,0);
                ::SendMessage(curScintilla,SCI_SETSELECTION,0,::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");

                sqlite3_stmt *stmt;
            
                bool notOverWrite = false;

                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                {
                    sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                    if(SQLITE_ROW == sqlite3_step(stmt))
                    {
                        sqlite3_finalize(stmt);
                        // TODO: may be moving the message to earlier location so that the text editor will be showing the message that is about to be overwriting into the database
                        // TODO: try showing the conflict message on the editor

                        ::SendMessage(curScintilla,SCI_GOTOLINE,0,0);
  
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\nConflicting Snippet: \r\n\r\n     ");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagText);
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"  <");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagTypeText);
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)">\r\n");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n   (More details of the conflicts will be shown in future releases)");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n----------------------------------------\r\n");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"---------- [ Pending Imports ] ---------\r\n");
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"----------------------------------------\r\n");

                        int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("A snippet already exists, overwrite?"), TEXT("FingerText"), MB_YESNO);
                        if (messageReturn==IDNO)
                        {
                            //delete [] tagText;
                            //delete [] tagTypeText;
                            //delete [] snippetText;
                            // not overwrite
                            //::MessageBox(nppData._nppHandle, TEXT("The Snippet is not saved."), TEXT("FingerText"), MB_OK);
                            notOverWrite = true;
            
                        } else
                        {
                            // delete existing entry
                            if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                            {
                                sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
                                sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                                sqlite3_step(stmt);
                        
                            } else
                            {
                                ::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT("FingerText"), MB_OK);
                            }
                    
                        }
                        ::SendMessage(curScintilla,SCI_GOTOLINE,17,0);
                        ::SendMessage(curScintilla,SCI_SETSELECTION,0,::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
                        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
                    } else
                    {
                        sqlite3_finalize(stmt);
                    }
                }
            
                if (notOverWrite == false && g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
                {
                    importCount++;
                    // Then bind the two ? parameters in the SQLite SQL to the real parameter values
                    sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
            
                    // Run the query with sqlite3_step
                    sqlite3_step(stmt); // SQLITE_ROW 100 sqlite3_step() has another row ready
                    //::MessageBox(nppData._nppHandle, TEXT("The Snippet is saved."), TEXT("FingerText"), MB_OK);
                }
                sqlite3_finalize(stmt);
                delete [] tagText;
                delete [] tagTypeText;
                delete [] snippetText;
            
                ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
                updateDockItems(false,false);
            
                ::SendMessage(curScintilla,SCI_GOTOPOS,0,0);
                ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
                next = ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"!$[FingerTextData FingerTextData]@#");

            } while (next>=0);
        
            wchar_t importCountText[35] = TEXT("");

            if (importCount>1)
            {
                ::_itow_s(importCount, importCountText, 10, 10);
                wcscat(importCountText,TEXT(" snippets are imported."));
            } else if (importCount==1)
            {
                wcscat(importCountText,TEXT("1 snippet is imported."));
            } else
            {
                wcscat(importCountText,TEXT("Snippet import is aborted."));
          
            }
            //::MessageBox(nppData._nppHandle, TEXT("Complete importing snippets"), TEXT("FingerText"), MB_OK);
            ::MessageBox(nppData._nppHandle, importCountText, TEXT("FingerText"), MB_OK);

            ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);

            //updateMode();
            //updateDockItems();
        }
        delete [] fileText;
    }

    if (g_liveHintUpdate == 3)
    {
        g_liveHintUpdate = 2;
    } else 
    {
        g_liveHintUpdate = 1;
    }
}

void promptSaveSnippet(TCHAR* message)
{
    HWND curScintilla = ::getCurrentScintilla();

    if ((message == NULL) && (g_editorView))
    {
        saveSnippet();
    } else if (::SendMessage(curScintilla,SCI_GETMODIFY,0,0)!=0)
    {
        //TODO: can be merged with the function promptsavesnippet();
        int messageReturn=::MessageBox(nppData._nppHandle, message, TEXT("FingerText"), MB_YESNO);
        if (messageReturn==IDYES)
        {
            saveSnippet();
        }
    }
}

void updateMode()
{
    HWND curScintilla = getCurrentScintilla();

    int curPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
    
    ::SendMessage(curScintilla,SCI_GOTOPOS,0,0);
    ::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
    ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"- FingerText Snippet Editor View -");
    
    if ((::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0) == 5) && (::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0) == 39))
    {
        snippetDock.toggleSave(true);
        g_editorView = true;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("EDIT MODE\r\n(Double click item in list to edit another snippet, Ctrl+S to save)\r\nList of All Snippets"));
    } else if (g_enable)
    {
        snippetDock.toggleSave(false);
        g_editorView = false;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Enabled]\r\n(Type trigger text and hit tab to insert snippet)\r\nList of Available Snippets"));
        
    } else
    {
        snippetDock.toggleSave(false);
        g_editorView = false;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Disabled]\r\n(To enable: Plugins>FingerText>Toggle FingerText On/Off)\r\nList of Available Snippets"));

    }
    ::SendMessage(curScintilla,SCI_GOTOPOS,curPos,0);
}


void showHelp()
{
    ::MessageBox(nppData._nppHandle, HELP_TEXT_FULL, TEXT("FingerText"), MB_OK);

     //ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/erinata/FingerText"), NULL, NULL, SW_SHOWNORMAL);
}

void showAbout()
{
    ::MessageBox(nppData._nppHandle, VERSION_TEXT_FULL, TEXT("FingerText"), MB_OK);
}

void keyUpdate()
{
    if (g_editorView)
    {
        refreshAnnotation();
    } else
    {
        snippetHintUpdate();
    }
}

void refreshAnnotation()
{
    if (g_editorView)
    {
        HWND curScintilla = getCurrentScintilla();

        ::SendMessage(curScintilla, SCI_ANNOTATIONCLEARALL, 0, 0);

        ::SendMessage(curScintilla, SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
                # The first line in this document should always be this    \r\n\
                # ------ FingerText Snippet Editor View ------ \r\n\
                # line. Don't mess it up! \r\n\r\n");
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETTEXT, 1, (LPARAM)"\
                # The second part is the trigger text. For example if you \r\n\
                # put \"npp\" (without quotes) in this line, the snippet will\r\n\
                # be triggered when you type npp and hit tab.\r\n\
                # Trigger text with more than 30 characters is NOT recommended\r\n\
                # Only alphanumerics are allowed.\r\n\r\n");
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETTEXT, 2, (LPARAM)"\
                # The third part is the scope of the snippet. \r\n\
                # e.g. \"GLOBAL\" (without quotes) for globally available\r\n\
                # snippets, and \".cpp\" (without quotes) for snippets that  \r\n\
                # is only available in .cpp documents.\r\n\r\n\r\n\r\n\
                # Anywhere below here is the snippet content. It can be\r\n\
                # as long as many paragraphs or just several words.\r\n\
                # Remember to place an [>END<] at the end of the snippet\r\n\
                # content.\r\n");
    
    
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETSTYLE, 0, STYLE_INDENTGUIDE);
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETSTYLE, 1, STYLE_INDENTGUIDE);
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETSTYLE, 2, STYLE_INDENTGUIDE);
    
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETVISIBLE, 2, 0);

    }

    
}

bool triggerTag(int &posCurrent, int triggerLength)
{
    HWND curScintilla = getCurrentScintilla();
    bool tagFound = false;
    char *tag;
	int tagLength = getCurrentTag(curScintilla, posCurrent, &tag, triggerLength);
    int posBeforeTag=posCurrent-tagLength;
    if (tagLength != 0)
	{
        char *expanded;

        int level=1;
        do
        {
            expanded = findTagSQLite(tag,level,TEXT(""),false); 
			if (expanded)
            {
                replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
				tagFound = true;
                break;
            } 
            level++;
        } while (level<=3);

        delete [] expanded;

		delete [] tag;
        // return to the original path 
        // ::SetCurrentDirectory(curPath);
    }
    // return to the original position 
    if (tagFound) ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);

    return tagFound;
}
void tagComplete()
{
    HWND curScintilla = getCurrentScintilla();
    int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    snippetComplete(posCurrent);
}

bool snippetComplete(int &posCurrent)
{
    HWND curScintilla = getCurrentScintilla();

    bool tagFound = false;
    char *tag;
	int tagLength = getCurrentTag(curScintilla, posCurrent, &tag);
    int posBeforeTag=posCurrent-tagLength;
    if (tagLength != 0)
	{
        char *expanded;

        int level=1;
        do
        {
            expanded = findTagSQLite(tag,level,TEXT(""),true); 
			if (expanded)
            {
                ::SendMessage(curScintilla,SCI_SETSEL,posBeforeTag,posCurrent);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)expanded);
                posBeforeTag = posBeforeTag+strlen(expanded);
                

                //replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
				tagFound = true;
                break;
            } 
            level++;
        } while (level<=3);

        delete [] expanded;

		delete [] tag;
        // return to the original path 
        // ::SetCurrentDirectory(curPath);
    }
    // return to the original position 
    if (tagFound) ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);

    return tagFound;

}

void fingerText()
{
    //bool preserveSteps=false;


    HWND curScintilla = getCurrentScintilla();

    if ((g_enable==false) || (g_editorView==true) || (::SendMessage(curScintilla,SCI_SELECTIONISRECTANGLE,0,0)==1))
    {
        ::SendMessage(curScintilla,SCI_TAB,0,0);	
    } else
    {
        if (g_liveHintUpdate == 1)  // Temporary turn off live update
        {
            g_liveHintUpdate = 2;
        } else 
        {
            g_liveHintUpdate = 3;
        }


        if (g_preserveSteps==1) 
        {
            ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
        }
        
        ::SendMessage(curScintilla,SCI_AUTOCCANCEL,0,0);	 
        int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        int posSelectionStart = ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
        int posSelectionEnd = ::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0);

        bool tagFound = false;
        if (posSelectionStart==posSelectionEnd)
        {
            tagFound = triggerTag(posCurrent);
        }

        posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

        //dynamic hotspot (chain snippet)
        chainSnippet(curScintilla, posCurrent);

        //dynamic hotspot (keyword spot)
        keyWordSpot(curScintilla, posCurrent);
        if (g_preserveSteps==1) 
	    {
		    ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	    }

        	  
        bool spotFound = hotSpotNavigation(curScintilla);

        


        bool completeFound = false;
        if (g_tabTagCompletion == 1)
        {
            if ((spotFound == false) && (tagFound == false)) 
		    {
                ::SendMessage(curScintilla, SCI_GOTOPOS, posCurrent, 0);
                completeFound = snippetComplete(posCurrent);
		    }
        }
        

        if ((spotFound == false) && (tagFound == false) && (completeFound==false)) 
        {
            restoreTab(curScintilla, posCurrent, posSelectionStart, posSelectionEnd);
        }

        
        if (g_liveHintUpdate == 3)
        {
            g_liveHintUpdate = 2;
        } else 
        {
            g_liveHintUpdate = 1;
        }

    } 

}

void testing()
{

    ::MessageBox(nppData._nppHandle, TEXT("Testing!"), TEXT("Trace"), MB_OK);
    
    HWND curScintilla = getCurrentScintilla();

    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    //int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    //snippetComplete(posCurrent);
    

    //
    //if (g_newUpdate)
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("New!"), TEXT("Trace"), MB_OK);
    //} else
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("Old!"), TEXT("Trace"), MB_OK);
    //}
    //
    //
    //::SendMessage(curScintilla, SCI_SETLINEINDENTATION, 0, 11);
    //::SendMessage(curScintilla, SCI_SETLINEINDENTATION, 1, 11);
    //::SendMessage(curScintilla, SCI_SETLINEINDENTATION, 2, 11);
    //::SendMessage(curScintilla, SCI_SETLINEINDENTATION, 3, 11);

    //int enc = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, 0, 0);
    //wchar_t countText[10];
    //::_itow_s(enc, countText, 10, 10); 
    //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);

    //    
    ////char *tagType1 = NULL;
    //TCHAR fileType1[5];
    //::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType1);
    ////convertToUTF8(fileType1, &tagType1);
    //::MessageBox(nppData._nppHandle, fileType1, TEXT("Trace"), MB_OK);
    //
    //TCHAR key[MAX_PATH];
    //::swprintf(key,fileType1);
    //::MessageBox(nppData._nppHandle, key, TEXT("Trace"), MB_OK);
    //
    //const TCHAR* key2 = (TCHAR*)".txt";
    //
    //if (key==key2)
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("txt!"), TEXT("Trace"), MB_OK);
    //
    //} else
    //{
    //   ::MessageBox(nppData._nppHandle, TEXT("not txt!"), TEXT("Trace"), MB_OK);
    //}
    //
    //::SendMessage(curScintilla, SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"Hello!");
    //::SendMessage(curScintilla, SCI_ANNOTATIONSETVISIBLE, 2, 0);
      
    //exportSnippets();

    // messagebox shows the current buffer encoding id
    //int enc = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERENCODING, (LPARAM)::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0), 0);

    //int enc = g_snippetListLength;
    //wchar_t countText[10];
    //::_itow_s(enc, countText, 10, 10); 
    //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);


    //TCHAR file2switch[]=TEXT("C:\\Users\\tomtom\\Desktop\\FingerTextEditor");
    //TCHAR file2switch[]=TEXT("FingerTextEditor");
    //::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)file2switch);

    

    //::SendMessage(nppData._nppHandle, NPPM_ACTIVATEDOC, MAIN_VIEW, 1);
    
    //::SendMessage(curScintilla, SCI_SETSELECTION, 0, 44);
    //
    //char selText[45];
    //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, (LPARAM)&selText);
    //
    //char key[]="------ FingerText Snippet Editor View ------";
    //
    ////if (selText == "------ FingerText Snippet Editor View ------")
    //if (::strcmp(selText,key))
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("true"), TEXT("Trace"), MB_OK);
    //} else
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("false"), TEXT("Trace"), MB_OK);
    //}
    //


        
    //::SendMessage(curScintilla,SCI_GOTOPOS,0,0);
    //::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
    //::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)"FingerText Snippet Editor View");
    //    
    //if (::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0) == 7)
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("true"), TEXT("Trace"), MB_OK);
    //} else
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("false"), TEXT("Trace"), MB_OK);
    //}

}

void alert()
{
     ::MessageBox(nppData._nppHandle, TEXT("Alert!"), TEXT("Trace"), MB_OK);
}