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
#include <fstream>

#include "PluginDefinition.h"
#include "menuCmdID.h"
#include "sqlite3.h"
#include "SnippetDock.h"
#include "Version.h"

//#include <process.h>
//#include <string.h>

// The plugin data that Notepad++ needs
FuncItem funcItem[nbFunc];
// The data of Notepad++ that you can use in your plugin commands
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
TCHAR g_fttempPath[MAX_PATH];
//TCHAR g_groupPath[MAX_PATH];

SnipIndex* g_snippetCache;
//int g_snippetCacheSize;


int g_version;

bool g_newUpdate;
bool g_modifyResponse;
bool g_enable;
bool g_editorView;

// For option hotspot
bool g_optionMode;
//bool g_optionOperating;
int g_optionStartPosition;
int g_optionEndPosition;
int g_optionCurrent;
int g_optionNumber;
//char *g_optionArray[] = {"","","","","","","","","","","","","","","","","","","",""};
char *g_optionArray[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
// Config file content
#define DEFAULT_SNIPPET_LIST_LENGTH 100
#define DEFAULT_SNIPPET_LIST_ORDER_TAG_TYPE 1
#define DEFAULT_TAB_TAG_COMPLETION 0
#define DEFAULT_LIVE_HINT_UPDATE 1
#define DEFAULT_INDENT_REFERENCE 1
#define DEFAULT_CHAIN_LIMIT 20
#define DEFAULT_PRESERVE_STEPS 0
#define DEFAULT_ESCAPE_CHAR 0
#define DEFAULT_IMPORT_OVERWRITE_OPTION 0
#define DEFAULT_IMPORT_OVERWRITE_CONFIRM 0
#define DEFAULT_INCLUSIVE_TRIGGERTEXT_COMPLETION 1
#define DEFAULT_LIVE_PREVIEW_BOX 1

#define DEFAULT_CUSTOM_SCOPE TEXT("")
#define DEFAULT_CUSTOM_ESCAPE_CHAR TEXT("")

int g_snippetListLength;
int g_snippetListOrderTagType;
int g_tabTagCompletion;
int g_liveHintUpdate;
int g_indentReference;
int g_chainLimit;
int g_preserveSteps;
//int g_escapeChar;
//int g_importOverWriteOption;
int g_importOverWriteConfirm;
int g_inclusiveTriggerTextCompletion;
int g_livePreviewBox;

TCHAR* g_customEscapeChar;
TCHAR* g_customScope;

DockingDlg snippetDock;

#define TRIGGER_SNIPPET_INDEX 0
#define WARMSPOT_NAVIGATION_INDEX 1
#define SNIPPET_DOCK_INDEX 2
#define TOGGLE_ENABLE_INDEX 3
#define SELECTION_TO_SNIPPETS_INDEX 4
#define IMPORT_SNIPPETS_INDEX 5
#define EXPORT_SNIPPETS_INDEX 6
#define EXPORT_AND_CLEAR_INDEX 7
//#define SEPARATOR_ONE_INDEX 8
#define TAG_COMPLETE_INDEX 9
//#define SEPARATOR_TWO_INDEX 10
#define INSERT_HOTSPOT_SIGN_INDEX 11
//#define INSERT_WARMSPOT_SIGN_INDEX 11
//#define INSERT_CHAIN_SIGN_INDEX 10
//#define INSERT_KEY_SIGN_INDEX 11
//#define INSERT_COMMAND_SIGN_INDEX 12
//#define SEPARATOR_THREE_INDEX 12
#define SETTINGS_INDEX 12
#define HELP_INDEX 13
#define ABOUT_INDEX 14
//#define SEPARATOR_FOUR_INDEX 16
#define TESTING_INDEX 17

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
    g_liveHintUpdate = 0;
    //saveCustomScope();
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

    //ShortcutKey *shKey2 = new ShortcutKey;
	//shKey2->_isAlt = false;
	//shKey2->_isCtrl = true;
	//shKey2->_isShift = false;
	//shKey2->_key = VK_F4;

    setCommand(TRIGGER_SNIPPET_INDEX, TEXT("Trigger Snippet/Navigate to Hotspot"), fingerText, shKey, false);
    
    //setCommand(WARMSPOT_NAVIGATION_INDEX, TEXT("Navigate to Warmspot"), goToWarmSpot, shKey2, false);
    setCommand(SNIPPET_DOCK_INDEX, TEXT("Toggle On/off SnippetDock"), showSnippetDock, NULL, false);
    setCommand(TOGGLE_ENABLE_INDEX, TEXT("Toggle On/Off FingerText"), toggleDisable, NULL, false);
    setCommand(SELECTION_TO_SNIPPETS_INDEX, TEXT("Create Snippet from Selection"),  selectionToSnippet, NULL, false);
    setCommand(IMPORT_SNIPPETS_INDEX, TEXT("Import Snippets"), importSnippets, NULL, false);
    setCommand(EXPORT_SNIPPETS_INDEX, TEXT("Export Snippets"), exportSnippetsOnly, NULL, false);
    setCommand(EXPORT_AND_CLEAR_INDEX, TEXT("Export and Delete All Snippets"), exportAndClearSnippets, NULL, false);
    //setCommand(SEPARATOR_ONE_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(TAG_COMPLETE_INDEX, TEXT("TriggerText Completion"), tagComplete, NULL, false);
    //setCommand(SEPARATOR_TWO_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(INSERT_HOTSPOT_SIGN_INDEX, TEXT("Insert a hotspot"), insertHotSpotSign, NULL, false);
    //setCommand(INSERT_WARMSPOT_SIGN_INDEX, TEXT("Insert a warmspot"), insertWarmSpotSign, NULL, false);
    //setCommand(INSERT_CHAIN_SIGN_INDEX, TEXT("Insert a dynamic hotspot (Chain snippet)"), insertChainSnippetSign, NULL, false);
    //setCommand(INSERT_KEY_SIGN_INDEX, TEXT("Insert a dynamic hotspot (Keyword Spot)"), insertKeyWordSpotSign, NULL, false);
    //setCommand(INSERT_COMMAND_SIGN_INDEX, TEXT("Insert a dynamic hotspot (Command)"), insertCommandLineSign, NULL, false);
    //setCommand(SEPARATOR_THREE_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(SETTINGS_INDEX, TEXT("Settings"), settings, NULL, false);
    setCommand(HELP_INDEX, TEXT("Quick Guide"), showHelp, NULL, false);
    setCommand(ABOUT_INDEX, TEXT("About"), showAbout, NULL, false);
    //setCommand(SEPARATOR_FOUR_INDEX, TEXT("---"), NULL, NULL, false);
    setCommand(TESTING_INDEX, TEXT("Testing"), testing, NULL, false);
}

// Here you can do the clean up (especially for the shortcut)
void commandMenuCleanUp()
{
    //delete [] snippetEditTemplate;
    delete funcItem[0]._pShKey;
	// Don't forget to deallocate your shortcut here
}

// This function help you to initialize your plugin commands
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

void initialize()
{
    HWND curScintilla = getCurrentScintilla();
    ::SendMessage(curScintilla,SCI_SETMULTIPASTE,1,0); 
}

void toggleDisable()
{
    if (g_enable)
    {
        ::MessageBox(nppData._nppHandle, TEXT("FingerText is disabled"), TEXT("FingerText"), MB_OK);
        g_enable = false;
    } else
    {
        ::MessageBox(nppData._nppHandle, TEXT("FingerText is enabled"), TEXT("FingerText"), MB_OK);
        g_enable = true;
    }
    updateMode();
}

//char *getGroupScope(TCHAR* group, int position)
//{
//    char *scope = NULL;
//    char *scopeListConverted = NULL;
//    TCHAR *scopeList = new TCHAR[MAX_PATH];
//
//    GetPrivateProfileString(TEXT("Snippet Group"), group,NULL,scopeList,MAX_PATH,g_groupPath);
//    convertToUTF8(scopeList,&scopeListConverted);
//
//    scope = strtok(scopeListConverted, "|");
//    while (position>0 && scope!=NULL) 
//    {
//        scope = strtok(NULL, "|");
//        position--;
//    } 
//    
//    delete [] scopeList;
//    return scope;
//
//}

char *findTagSQLite(char *tag, char *tagCompare, bool similar=false)
{
    //alertCharArray(tagCompare);
	char *expanded = NULL;
	sqlite3_stmt *stmt;

    // First create the SQLite SQL statement ("prepare" it for running)
    char *sqlitePrepareStatement;
    if (similar)
    {
        sqlitePrepareStatement = "SELECT tag FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag";
    } else
    {
        sqlitePrepareStatement = "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag";
    }
    
    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, sqlitePrepareStatement, -1, &stmt, NULL))
	{
        sqlite3_bind_text(stmt, 1, tagCompare, -1, SQLITE_STATIC);

        if (similar)
        {
            char similarTag[MAX_PATH]="";
            if (g_inclusiveTriggerTextCompletion==1) strcat(similarTag,"%");
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

//char *findTagSQLiteBackup(char *tag, int level, bool similar=false)
//{
//	char *expanded = NULL;
//	sqlite3_stmt *stmt;
//
//    // First create the SQLite SQL statement ("prepare" it for running)
//    char *sqlitePrepareStatement;
//    if (similar == false)
//    {
//        sqlitePrepareStatement = "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag";
//    } else
//    {
//        sqlitePrepareStatement = "SELECT tag FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag";
//    }
//    
//    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, sqlitePrepareStatement, -1, &stmt, NULL))
//	{
//        char *tagType = NULL;
//   
//        TCHAR *fileType = NULL;
//        fileType = new TCHAR[MAX_PATH];
//        ::swprintf(fileType,TEXT("GLOBAL"));
//        if (level == 1)
//        {
//	        ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
//        } else if (level == 2)
//        {
//	        ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
//        }
//        convertToUTF8(fileType, &tagType);
//        delete [] fileType;
// 
//		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
//		sqlite3_bind_text(stmt, 1, tagType, -1, SQLITE_STATIC);
//
//        if (similar==1)
//        {
//            char similarTag[MAX_PATH]="";
//            if (g_inclusiveTriggerTextCompletion==1) strcat(similarTag,"%");
//            strcat(similarTag,tag);
//            strcat(similarTag,"%");
//            //::SendMessage(getCurrentScintilla(),SCI_INSERTTEXT,0,(LPARAM)similarTag);
//            sqlite3_bind_text(stmt, 2, similarTag, -1, SQLITE_STATIC);
//        } else
//        {
//		    sqlite3_bind_text(stmt, 2, tag, -1, SQLITE_STATIC);
//        }
//
//		// Run the query with sqlite3_step
//		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
//		{
//			const char* expandedSQL = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
//			expanded = new char[strlen(expandedSQL)*4 + 1];
//			strcpy(expanded, expandedSQL);
//		}
//	}
//    // Close the SQLite statement, as we don't need it anymore
//	// This also has the effect of free'ing the result from sqlite3_column_text 
//	// (i.e. in our case, expandedSQL)
//	sqlite3_finalize(stmt);
//	return expanded; //remember to delete the returned expanded after use.
//}

void upgradeMessage()
{
    //TODO: dynamic upgrade message
    HWND curScintilla = getCurrentScintilla();
    if (g_newUpdate)
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
        ::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)WELCOME_TEXT);
    }
}

int searchNext(HWND &curScintilla, char* searchText)
{
    ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
    return ::SendMessage(curScintilla, SCI_SEARCHNEXT, 0,(LPARAM)searchText);
}
int searchPrev(HWND &curScintilla, char* searchText)
{
    ::SendMessage(curScintilla, SCI_SEARCHANCHOR, 0,0);
    return ::SendMessage(curScintilla, SCI_SEARCHPREV, 0,(LPARAM)searchText);
}

void selectionToSnippet()
{
    HWND curScintilla = getCurrentScintilla();
    int selectionEnd = ::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0);
    int selectionStart = ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
    bool withSelection = false;

    char* selection;
    
    if (selectionEnd>selectionStart)
    {
        selection = new char [selectionEnd - selectionStart +1];
        ::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        withSelection = true;
    } else
    {
        selection = "New snippet is here.\r\nNew snippet is here.\r\nNew snippet is here.\r\n";
    }

    //::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
    
    if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
    {
        ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_ftbPath);
    } 

    curScintilla = getCurrentScintilla();

    //TODO: consider using YES NO CANCEL dialog in promptsavesnippet
    promptSaveSnippet(TEXT("Do you wish to save the current snippet before creating a new one?"));
    
    ::SendMessage(curScintilla,SCI_CLEARALL,0,0);

    ::SendMessage(curScintilla,SCI_INSERTTEXT,::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)SNIPPET_EDIT_TEMPLATE);
    ::SendMessage(curScintilla,SCI_INSERTTEXT,::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)"triggertext\r\nGLOBAL\r\n");
    ::SendMessage(curScintilla,SCI_INSERTTEXT,::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)selection);
    ::SendMessage(curScintilla,SCI_INSERTTEXT,::SendMessage(curScintilla, SCI_GETLENGTH,0,0), (LPARAM)"[>END<]");

    g_editorView = 1;    
    updateDockItems(false,false);
    //updateMode();
    refreshAnnotation();
    ::SendMessage(curScintilla,SCI_GOTOLINE,1,0);
    ::SendMessage(curScintilla,SCI_WORDRIGHTEXTEND,1,0);

    if (withSelection) delete [] selection;
}

void editSnippet()
{
    int index = snippetDock.getCount() - snippetDock.getSelection()-1;
    char* tempTriggerText;
    char* tempScope;
    //char * tempSnippetText;

    //tempTriggerText = new char [strlen(g_snippetCache[index].triggerText)];
    //tempScope = new char [strlen(g_snippetCache[index].scope)];

    //strcpy(tempTriggerText, g_snippetCache[index].triggerText);
    //strcpy(tempScope, g_snippetCache[index].scope);
    //sprintf(tempTriggerText, g_snippetCache[index].triggerText);
    //sprintf(tempScope, g_snippetCache[index].scope);
    //
    tempTriggerText = g_snippetCache[index].triggerText;
    tempScope = g_snippetCache[index].scope;

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
            HWND curScintilla = getCurrentScintilla();
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

    HWND curScintilla = getCurrentScintilla();
    ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
    ::SendMessage(curScintilla,SCI_EMPTYUNDOBUFFER,0,0);

    //if (tempTriggerText)
    //{
        //delete [] tempTriggerText; // deleting them cause error
    //}
    //if (tempScope) delete [] tempScope;
}

void deleteSnippet()
{
    //TODO: should scroll back to original position after delete
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
    // TODO: and check for more error, say the triggertext has to be one word
    // TODO: consolidate also the snippet content extraction of savesnippet

    bool problemSnippet = false;

    ::SendMessage(curScintilla,SCI_GOTOLINE,lineNumber,0);

    int tagPosStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    int tagPosEnd;
    
    if (lineNumber == 3)
    {
        tagPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
    } else
    {
        int tagPosLineEnd = ::SendMessage(curScintilla,SCI_GETLINEENDPOSITION,lineNumber,0);

        char* wordChar;
        if (lineNumber==2)
        {
            wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.:";
            
        } else //if (lineNumber==1)
        {
            wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";
        }
        ::SendMessage(curScintilla,	SCI_SETWORDCHARS, 0, (LPARAM)wordChar);
        tagPosEnd = ::SendMessage(curScintilla,SCI_WORDENDPOSITION,tagPosStart,0);
        ::SendMessage(curScintilla,	SCI_SETCHARSDEFAULT, 0, 0);
        //::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        //::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" ");
        //tagPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        if ((tagPosEnd>tagPosLineEnd) || (tagPosEnd-tagPosStart<=0))
        {
            //blank
            ::SendMessage(curScintilla,SCI_GOTOLINE,lineNumber,0);
            ::MessageBox(nppData._nppHandle, errorText, TEXT("FingerText"), MB_OK);
            problemSnippet = true;
            
        } else if (tagPosEnd<tagPosLineEnd)
        {
            // multi
            ::SendMessage(curScintilla,SCI_GOTOLINE,lineNumber,0);
            ::MessageBox(nppData._nppHandle, errorText, TEXT("FingerText"), MB_OK);
            problemSnippet = true;
        }
    }

    if (lineNumber == 3)
    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,tagPosStart,0);
        int spot = searchNext(curScintilla, "[>END<]");
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
    // TODO: Make sure that it is not necessary to keep this line
    //::SendMessage(curScintilla, SCI_INSERTTEXT, docLength, (LPARAM)" ");
    
    bool problemSnippet = false;

    char* tagText;
    char* tagTypeText;
    char* snippetText;

    if (getLineChecked(&tagText,curScintilla,1,TEXT("TriggerText cannot be blank, and it can only contain alphanumeric characters (no spaces allowed)"))==true) problemSnippet = true;
    if (getLineChecked(&tagTypeText,curScintilla,2,TEXT("Scope cannot be blank, and it can only contain alphanumeric characters and/or period."))==true) problemSnippet = true;
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

// TODO: refactor the dynamic hotspot functions
bool dynamicHotspot(HWND &curScintilla, int &startingPos)
{
    //char* tagSign;
    //if (hotSpotType == 1)
    //{
    //    tagSign = "$[![(cha)";
    //} else if (hotSpotType == 2)
    //{
    //    tagSign = "$[![(key)";
    //} else
    //{
    //    tagSign = "$[![(cmd)";
    //}
    int checkPoint = startingPos;    

    char tagSign[] = "$[![";
    //int tagSignLength = strlen(tagSign);
    int tagSignLength = 4;
    char tagTail[] = "]!]";
    //int tagTailLength = strlen(tagTail);
    
    //int tagSignLength = strlen(tagSign);
    //int tagSignLength = 4;
    char* hotSpotText;
    char* hotSpot;
    int spot = -1;
    int spotComplete = -1;
    int spotType = 0;

    int limitCounter = 0;
    do 
    {
        ::SendMessage(curScintilla,SCI_GOTOPOS,checkPoint,0);
        spot = searchNext(curScintilla, tagTail);   // Find the tail first so that nested snippets are triggered correctly
        //spot = searchNext(curScintilla, tagSign);
        if (spot>=0)
	    {
            checkPoint = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+1;
            spotComplete = -1;
            spotComplete = searchPrev(curScintilla, tagSign);
            if (spotComplete>=0)
            {

                int firstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                int secondPos = 0;
                spotType = grabHotSpotContent(curScintilla, &hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength,true);
                
                ///////////////////
                if (spotType == 1)
                {
                    checkPoint = firstPos;
                    chainSnippet(curScintilla, firstPos, hotSpotText+5);
                    
                    limitCounter++;
                } else if (spotType == 2)
                {
                    checkPoint = firstPos;
                    keyWordSpot(curScintilla, firstPos,hotSpotText+5, startingPos, checkPoint);
                    
                    limitCounter++;
                } else if (spotType == 3)
                {
                    checkPoint = firstPos;
                    executeCommand(curScintilla, firstPos, hotSpotText+5);
                    
                    limitCounter++;
                } else
                {
                    limitCounter++;
                }
            }
            //////////////////////
        }
        
    } while ((spotComplete>=0) && (spot>0) && (limitCounter<g_chainLimit));

    //TODO: loosen the limit to the limit of special spot, and ++limit for every search so that less frezze will happen
    if (limitCounter>=g_chainLimit) ::MessageBox(nppData._nppHandle, TEXT("Dynamic hotspots triggering limit exceeded."), TEXT("FingerText"), MB_OK);

    if (limitCounter>0)
    {
        delete [] hotSpot;
        delete [] hotSpotText;

        return true;
    }
    return false;
}

void chainSnippet(HWND &curScintilla, int &firstPos, char* hotSpotText)
{
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendMessage(curScintilla,SCI_GOTOPOS,triggerPos,0);
    triggerTag(triggerPos,false, strlen(hotSpotText));
}

void executeCommand(HWND &curScintilla, int &firstPos, char* hotSpotText)
{
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendMessage(curScintilla,SCI_SETSEL,firstPos,triggerPos);
    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
    
    char  psBuffer[130];
    FILE   *pPipe;
    int resultLength;
    //TODO: try the createprocess instead of _popen?
    //http://msdn.microsoft.com/en-us/library/ms682499(v=vs.85).aspx

    //pPipe = _popen( "ruby -e 'puts 1+1'", "rt" );
    if( (pPipe = _popen( hotSpotText, "rt" )) == NULL )
    {    
        return;
    }

    ::memset(psBuffer,0,sizeof(psBuffer));

    while(fgets(psBuffer, 129, pPipe))
    {
        ::SendMessage(curScintilla, SCI_REPLACESEL, 128, (LPARAM)psBuffer);
        ::memset (psBuffer,0,sizeof(psBuffer));
    }
    _pclose( pPipe );
}

void keyWordSpot(HWND &curScintilla, int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint)
{
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendMessage(curScintilla,SCI_SETSEL,firstPos,triggerPos);
    //TODO: At least I should rearrange the keyword a little bit for efficiency
    if (strcmp(hotSpotText,"PASTE")==0)
    {
        ::SendMessage(curScintilla,SCI_PASTE,0,0);
	    
    } else if (strcmp(hotSpotText,"DATE")==0)
    {
        char* dateText = getDateTime(NULL,true,DATE_LONGDATE);
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)dateText);
        delete [] dateText;
        //insertDateTime(true,DATE_LONGDATE,curScintilla);
        
    } else if (strcmp(hotSpotText,"TIME")==0)
    {
        char* timeText = getDateTime(NULL,false,0);
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)timeText);
        delete [] timeText;
        //insertDateTime(false,0,curScintilla);
    } else if (strcmp(hotSpotText,"FILENAME")==0)
    {
        insertNppPath(NPPM_GETNAMEPART,curScintilla);
        
    } else if (strcmp(hotSpotText,"EXTNAME")==0)
    {
        insertNppPath(NPPM_GETEXTPART,curScintilla);
        
    } else if (strcmp(hotSpotText,"DIRECTORY")==0)
    {
        insertNppPath(NPPM_GETCURRENTDIRECTORY,curScintilla);
    //} else if (strcmp(hotSpotText,"GETSCRIPT")==0) 
    } else if (strcmp(hotSpotText,"TEMPFILE")==0)
    {
        insertPath(g_fttempPath,curScintilla);

    } else if (strncmp(hotSpotText,"GET:",4)==0) 
    {
        //TODO: write a function to get the command and parameter sepearately. or turn this whole thing into a new type of hotspot
        //TODO: refactor GET, GET:, GETALL, GETLINE, CUT, CUT:
        
        emptyFile(g_fttempPath);
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+4);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        int scriptFound = -1;
        if (strlen(getTerm)>0)
        {
            ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
            scriptFound = searchPrev(curScintilla,getTerm);
        }
        delete [] getTerm;

        int scriptStart = 0;
        if (scriptFound>=0)
        {
            scriptStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        }
            
        int selectionEnd = startingPos-1; // -1 because the space before the snippet tag should not be included
        int selectionStart = scriptStart+strlen(hotSpotText)-4;
        if (selectionEnd>=selectionStart)
        {
            ::SendMessage(curScintilla,SCI_SETSEL,selectionStart,selectionEnd);
            char* selection = new char [selectionEnd - selectionStart +1];
            ::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
            ::SendMessage(curScintilla,SCI_SETSEL,scriptStart,selectionEnd+1); //+1 to make up the -1 in setting the selection End
            ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
            startingPos = startingPos - (selectionEnd - scriptStart + 1); //+1 to make up the -1 in setting the selection End
            checkPoint = checkPoint - (selectionEnd - scriptStart + 1); //+1 to make up the -1 in setting the selection End
            std::ofstream myfile(g_fttempPath, std::ios::binary); // need to open in binary so that there will not be extra spaces written to the document
            if (myfile.is_open())
            {
                myfile << selection;
                myfile.close();
            }
            delete [] selection;
        } else
        {
            alertCharArray("keyword GET: caused an error.");
        }
    } else if (strcmp(hotSpotText,"GET")==0)
    {
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_SETSEL,startingPos-1,startingPos);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
        ::SendMessage(curScintilla,SCI_WORDLEFTEXTEND,0,0);
        char* selection = new char [startingPos -1 - ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0) +1];
        ::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        startingPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        std::ofstream myfile(g_fttempPath, std::ios::binary);
        if (myfile.is_open())
        {
            myfile << selection;
            myfile.close();
        }
        delete [] selection;

    } else if (strcmp(hotSpotText,"GETLINE")==0)
    {
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_SETSEL,startingPos-1,startingPos);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
        ::SendMessage(curScintilla,SCI_HOMEEXTEND,0,0);
        char* selection = new char [startingPos -1 - ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0) +1];
        ::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        startingPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        std::ofstream myfile(g_fttempPath, std::ios::binary);
        if (myfile.is_open())
        {
            myfile << selection;
            myfile.close();
        }
        delete [] selection;

    } else if (strncmp(hotSpotText,"CUT:",4)==0) 
    {
        emptyFile(g_fttempPath);
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+4);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        int scriptFound = -1;
        if (strlen(getTerm)>0)
        {
            ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
            scriptFound = searchPrev(curScintilla,getTerm);
        }
        delete [] getTerm;
        // TODO: still a bug when triggered from the beginning of the document with a searching term. For example: $[![(key)CUT: ]!]$[![(key)SCOPE:$[![(key)PASTE]!]]!][>END<] (testscope) will get 1 missing character when triggering from start of the document
        // TODO: same problem for GET:
        int scriptStart = 0;
        if (scriptFound>=0)
        {
            scriptStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        }
        int selectionEnd = startingPos-1; // -1 because the space before the snippet tag should not be included
        int selectionStart = scriptStart+strlen(hotSpotText)-4;
        if (selectionEnd>=selectionStart)
        {
            ::SendMessage(curScintilla,SCI_SETSEL,selectionStart,selectionEnd);
            ::SendMessage(curScintilla,SCI_COPY,0,0);
            ::SendMessage(curScintilla,SCI_SETSEL,scriptStart,selectionEnd+1); //+1 to make up the -1 in setting the selection End
            ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
            startingPos = startingPos - (selectionEnd - scriptStart + 1); //+1 to make up the -1 in setting the selection End
            checkPoint = checkPoint - (selectionEnd - scriptStart + 1); //+1 to make up the -1 in setting the selection End
        } else
        {
            //alertNumber(selectionStart);
            //alertNumber(selectionEnd);
            alertCharArray("keyword CUT: caused an error.");
        }
            
        
    } else if (strcmp(hotSpotText,"CUT")==0)
    {
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_SETSEL,startingPos-1,startingPos);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
        ::SendMessage(curScintilla,SCI_WORDLEFTEXTEND,0,0);
        startingPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        ::SendMessage(curScintilla,SCI_CUT,0,0);
    } else if (strcmp(hotSpotText,"CUTLINE")==0)
    {
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_SETSEL,startingPos-1,startingPos);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        ::SendMessage(curScintilla,SCI_GOTOPOS,startingPos-1,0);
        ::SendMessage(curScintilla,SCI_HOMEEXTEND,0,0);
        startingPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        ::SendMessage(curScintilla,SCI_CUT,0,0);
    } else if (strncmp(hotSpotText,"UPPER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+6);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)::strupr(getTerm));
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"LOWER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+6);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)::strlwr(getTerm));
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"TIME:",5)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+5);
        char* timeReturn = getDateTime(getTerm,false);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)timeReturn);
        delete [] timeReturn;
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"DATE:",5)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+5);
        char* dateReturn = getDateTime(getTerm);
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)dateReturn);
        delete [] dateReturn;
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"SCOPE:",6)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+6);
        //TCHAR* scopeWide = new TCHAR[strlen(getTerm)*4+!];
        convertToWideChar(getTerm, &g_customScope);
        writeConfigTextChar(g_customScope,TEXT("custom_scope"));
        ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
        updateDockItems(false,false);
        //snippetHintUpdate();
        delete [] getTerm;
    } else if ((strncmp(hotSpotText,"CLEAN_",6)==0) && (strncmp(hotSpotText+7,":",1)==0))
    {
        // TODO: fill in content for this CLEAN keyword
        
    } else if ((strncmp(hotSpotText,"COUNT_",6)==0) && (strncmp(hotSpotText+7,":",1)==0))
    {
        // TODO: fill in content for this COUNT keyword
    }

    //else if (strcmp(hotSpotText,"DATESHORT")==0)
    //{
    //    char* dateText = getDateTime(NULL,true,DATE_SHORTDATE);
    //    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)dateText);
    //    delete [] dateText;
    //    //insertDateTime(true,DATE_SHORTDATE,curScintilla);
	//    
    //} 
    //else if (strcmp(hotSpotText,"TIMESHORT")==0)
    //{
    //    char* timeText = getDateTime(NULL,false,TIME_NOSECONDS);
    //    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)timeText);
    //    delete [] timeText;
    //    //insertDateTime(false,TIME_NOSECONDS,curScintilla);
    //    
    //} 
}

//TODO: insertpath and insertnpppath (and/or other insert function) need refactoring
void insertPath(TCHAR* path, HWND &curScintilla)
{
    char pathText[MAX_PATH];
	WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, path, -1, pathText, MAX_PATH, NULL, NULL);
    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)pathText);
}

void insertNppPath(int msg, HWND &curScintilla)
{
	TCHAR path[MAX_PATH];
	::SendMessage(nppData._nppHandle, msg, 0, (LPARAM)path);

	char pathText[MAX_PATH];
	WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, path, -1, pathText, MAX_PATH, NULL, NULL);
	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)pathText);
}

//void insertDateTime(bool date,int type, HWND &curScintilla)
//{
//    TCHAR time[128];
//    SYSTEMTIME formatTime;
//	::GetLocalTime(&formatTime);
//    if (date)
//    {
//        ::GetDateFormat(LOCALE_USER_DEFAULT, type, &formatTime, NULL, time, 128);
//    } else
//    {
//        ::GetTimeFormat(LOCALE_USER_DEFAULT, type, &formatTime, NULL, time, 128);
//    }
//	
//	char timeText[MAX_PATH];
//	WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, time, -1, timeText, MAX_PATH, NULL, NULL);
//	::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)timeText);
//}

char* getDateTime(char *format, bool getDate, int flags)
{
    HWND curScintilla = getCurrentScintilla();

    TCHAR result[128];
    SYSTEMTIME formatTime;
	::GetLocalTime(&formatTime);
    
    wchar_t* formatWide;
    convertToWideChar(format, &formatWide);

    if (getDate)
    {
        ::GetDateFormat(LOCALE_USER_DEFAULT, flags, &formatTime, formatWide, result, 128);
    } else
    {
        ::GetTimeFormat(LOCALE_USER_DEFAULT, flags, &formatTime, formatWide, result, 128);
    }
    
    if (format) delete [] formatWide;
    
    char* resultText;// = new char[MAX_PATH];
    convertToUTF8(result,&resultText);
	//WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, result, -1, resultText, MAX_PATH, NULL, NULL);
    return resultText;
}

//void goToWarmSpot()
//{
//    HWND curScintilla = getCurrentScintilla();
//    if (!warmSpotNavigation(curScintilla))
//    {
//        ::SendMessage(curScintilla,SCI_BACKTAB,0,0);	
//    }
//
//}
//
//bool warmSpotNavigation(HWND &curScintilla)
//{
//    
//    bool spotFound = searchNext(curScintilla,"${!{}!}");
//    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
//    return spotFound;
//}

bool hotSpotNavigation(HWND &curScintilla)
{
    // TODO: consolidate this part with dynamic hotspots? 

    char tagSign[] = "$[![";
    //int tagSignLength = strlen(tagSign);
    int tagSignLength = 4;
    char tagTail[] = "]!]";
    //int tagTailLength = strlen(tagTail);

    char *hotSpotText;
    char *hotSpot;

    int tagSpot = searchNext(curScintilla, tagTail);    // Find the tail first so that nested snippets are triggered correctly
	if (tagSpot>=0)
	{
        if (g_preserveSteps==0) ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);

        if (searchPrev(curScintilla, tagSign)>=0)
        {
            int firstPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
            int secondPos = 0;
            grabHotSpotContent(curScintilla, &hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength,false);
            
            ::SendMessage(curScintilla,SCI_GOTOPOS,firstPos,0);

            int tempOptionStart = 0;
            int tempOptionEnd = 0;

            if (strncmp(hotSpotText,"(opt)",5)==0)
            {
                //TODO: refactor the option hotspot part to a function
                
                cleanOptionItem();
                tempOptionEnd = firstPos + 5 - 3;
                int i =0;
                int optionFound = -1;
                while (i<20)
                {
                    tempOptionStart = tempOptionEnd + 3;
                    ::SendMessage(curScintilla,SCI_GOTOPOS,tempOptionStart,0);
                    optionFound = searchNext(curScintilla, "|~|");
                    if ((optionFound>=0) && (::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)<secondPos))
                    {
                        tempOptionEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                        ::SendMessage(curScintilla,SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                        char* optionText;
                        optionText = new char[tempOptionEnd - tempOptionStart + 1];
                        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                        addOptionItem(optionText);
                        i++;
                    } else
                    {
                        tempOptionEnd = secondPos-4;
                        ::SendMessage(curScintilla,SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                        char* optionText;
                        optionText = new char[tempOptionEnd - tempOptionStart + 1];
                        ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                        addOptionItem(optionText);
                        i++;
                        
                        break;
                    }
                };

                //g_optionOperating = true; 

                ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,tempOptionEnd);
                ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);
                //g_optionOperating = false; 

                ::SendMessage(curScintilla,SCI_GOTOPOS,firstPos,0);
                g_optionStartPosition = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                g_optionEndPosition = g_optionStartPosition + strlen(g_optionArray[g_optionCurrent]);
                ::SendMessage(curScintilla,SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
                if (i>1) g_optionMode = true;
                //alertNumber(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
                
                    
                //char* tempText;
                //tempText = new char[secondPos - firstPos + 1];
                //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(tempText));
                //alertCharArray(tempText);

                //::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos);
                //::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);

            } else
            {

                int hotSpotFound=-1;
                int tempPos[100];
                int i=1;
                //TODO: The hotspot with the same name cannot be next to each others. This will be fixed when scintilla updates and notepad++ adopt the changes.
                //TODO: consider refactor this part to another function
                for (i=1;i<=98;i++)
                {
                    tempPos[i]=-1;
                
                    hotSpotFound = searchNext(curScintilla, hotSpot);
                    if ((hotSpotFound>=0) && strlen(hotSpotText)>0)
                    {
                        tempPos[i] = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
                        ::SendMessage(curScintilla,SCI_GOTOPOS,tempPos[i],0);
                    } else
                    {
                        break;
                        //tempPos[i]=-1;
                    }
                }
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
            }

            delete [] hotSpot;
            delete [] hotSpotText;

            if (g_preserveSteps==0) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
            return true;
        }
	} else
    {
        //delete [] hotSpot;  // Don't try to delete if it has not been initialized
        //delete [] hotSpotText;
        return false;
    }
    return false;
}

int grabHotSpotContent(HWND &curScintilla, char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength, bool dynamic)
{
    //TODO: examine whether the bool dynamic is still needed, or we just use the (cha) (key) (cmd) and (opt) to determine what should happen
    
    int spotType = 0;

    searchNext(curScintilla, "]!]");
	secondPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos+signLength,secondPos);

    *hotSpotText = new char[secondPos - (firstPos + signLength) + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpotText));

    if (strncmp(*hotSpotText,"(cha)",5)==0)
    {
        spotType = 1;
        //alertCharArray("key");
    } else if (strncmp(*hotSpotText,"(key)",5)==0)
    {
        spotType = 2;
        //alertCharArray("cha");
    } else if (strncmp(*hotSpotText,"(cmd)",5)==0)
    {
        spotType = 3;
        //alertCharArray("cmd");
    }

    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos+3);
    
    *hotSpot = new char[secondPos+3 - firstPos + 1];
    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpot));

    if ((spotType>0) && (dynamic))
    {
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)*hotSpotText+5);
    } else if (!dynamic)
    {
        ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)*hotSpotText);

    }

    ::SendMessage(curScintilla,SCI_GOTOPOS,secondPos+3,0);
    return spotType;
    //return secondPos;  
}

//void grabHotSpotContentBackup(HWND &curScintilla, char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength)
//{
//    int posLine = ::SendMessage(curScintilla,SCI_LINEFROMPOSITION,0,0);
//        
//    searchNext(curScintilla, "]!]");
//	secondPos = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
//
//    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos+signLength,secondPos);
//
//    *hotSpotText = new char[secondPos - (firstPos + signLength) + 1];
//    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpotText));
//
//    ::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos+3);
//    
//    *hotSpot = new char[secondPos+3 - firstPos + 1];
//    ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpot));
//
//    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)*hotSpotText);
//    ::SendMessage(curScintilla,SCI_GOTOPOS,secondPos+3,0);
//    
//    //return secondPos;  
//}

void showPreview(bool top)
{
    //TODO: Testing using LB_SETANCHORINDEX LB_GETANCHORINDEX LB_SETCARETINDEX LB_GETCARETINDEX to change the selection in list box 
    sqlite3_stmt *stmt;
    int index = 0;

    HWND curScintilla = getCurrentScintilla();
    int posCurrent = ::SendMessage(curScintilla, SCI_GETCURRENTPOS,0,0);

    if (top)
    {
        index = snippetDock.getCount()-1;
    } else
    {
        index = snippetDock.getCount() - snippetDock.getSelection()-1;
    }

    //else
    //{
    //    index = snippetDock.getCount()-1;
    //}
    if (index >= 1)
    {
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
	    	}	
	    }
	    sqlite3_finalize(stmt);
    } else
    {
        
        snippetDock.setDlgText(ID_SNIPSHOW_EDIT,TEXT("Select an item in SnippetDock to view the snippet preview here."));
    }
    //::SendMessage(curScintilla,SCI_GRABFOCUS,0,0); 
}

void insertHotSpotSign()
{
    insertTagSign("$[![]!]");
}

//void insertWarmSpotSign()
//{
//    insertTagSign("${!{}!}");
//}
//void insertChainSnippetSign()
//{
//    insertTagSign("$[![(cha)snippetname]!]");
//}
//void insertKeyWordSpotSign()
//{
//    insertTagSign("$[![(key)somekeyword]!]");
//}
//void insertCommandLineSign()
//{
//    insertTagSign("$[![(cmd)somecommand]!]");
//}

void insertTagSign(char * tagSign)
{
    HWND curScintilla = getCurrentScintilla();
    int posStart = ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagSign);
    //int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
    ::SendMessage(curScintilla,SCI_GOTOPOS,posStart+4,0);
    //if (g_editorView)
    //{
        //HWND curScintilla = getCurrentScintilla();
        //int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        //::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent,0);
        //::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagSign);
        //::SendMessage(curScintilla,SCI_SETSEL,posCurrent+strlen(tagSign)-3-11,posCurrent+strlen(tagSign)-3);
        ////::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent+strlen(tagSign)-3,0);
    //} else
    //{
    //    ::MessageBox(nppData._nppHandle, TEXT("Hotspots can be inserted only when you are editing snippets."), TEXT("FingerText"), MB_OK);
    //}
}


bool replaceTag(HWND &curScintilla, char *expanded, int &posCurrent, int &posBeforeTag)
{
    
    //TODO: can use ::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line-1, 0); to make sure that caret is visible after long snippet substitution.

    //::MessageBox(nppData._nppHandle, TEXT("replace tag"), TEXT("Trace"), MB_OK); 
    //std::streamoff sniplength;
    int lineCurrent = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, posCurrent, 0);
    int initialIndent = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, lineCurrent, 0);

    ::SendMessage(curScintilla, SCI_INSERTTEXT, posCurrent, (LPARAM)"____`[SnippetInserting]");

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

    searchNext(curScintilla, "`[SnippetInserting]");
    int posEndOfInsertedText = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;

    // adjust indentation according to initial indentation
    if (g_indentReference==1)
    {
        int lineInsertedSnippet = ::SendMessage(curScintilla, SCI_LINEFROMPOSITION, posEndOfInsertedText, 0);

        int lineIndent=0;
        for (int i=lineCurrent+1;i<=lineInsertedSnippet;i++)
        {
            lineIndent = ::SendMessage(curScintilla, SCI_GETLINEINDENTATION, i, 0);
            ::SendMessage(curScintilla, SCI_SETLINEINDENTATION, i, initialIndent+lineIndent);
        }
    }
    searchNext(curScintilla, "`[SnippetInserting]");
    posEndOfInsertedText = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0)+19;
                    
    ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
    searchNext(curScintilla, "[>END<]");
    int posEndOfSnippet = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        
    ::SendMessage(curScintilla,SCI_SETSELECTION,posEndOfSnippet,posEndOfInsertedText);

    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)"");
	::SendMessage(curScintilla, SCI_GOTOPOS, posCurrent, 0);
    return true;
}


void pluginShutdown()  // function is triggered when NPPN_SHUTDOWN fires.
{   
    g_liveHintUpdate = 0;
    delete [] g_snippetCache;
    delete [] g_customScope;
    delete [] g_customEscapeChar;
    //if (g_newUpdate) writeConfig();
    if (g_dbOpen)
    {
        sqlite3_close(g_db);  // This close the database when the plugin shutdown.
        g_dbOpen = false;
    }
}

void setConfigAndDatabase()
{
    g_modifyResponse = true;
    g_enable = true;
    g_customScope = new TCHAR[MAX_PATH];
    g_customEscapeChar = new TCHAR[MAX_PATH];

    // For option hotspot
    g_optionMode = false;
    //g_optionOperating = false;
    g_optionStartPosition = 0;
    g_optionEndPosition = 0;
    g_optionCurrent = 0;
    g_optionNumber = 0;
    
    //g_customScope = "";
    //g_display=false;
    updateMode();
    
    TCHAR path[MAX_PATH];
    char cpath[MAX_PATH*2];
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(path));
    ::_tcscat(path,TEXT("\\FingerText"));
    int multibyteLength = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, 0, 0);
    //cpath = new char[multibyteLength + 50];
    WideCharToMultiByte(CP_UTF8, 0, path, -1, cpath, multibyteLength, 0, 0);
    strcat(cpath, "\\FingerText.db3");
    
    if (PathFileExists(path) == FALSE) ::CreateDirectory(path, NULL);
    
    int rc = sqlite3_open(cpath, &g_db);
    if (rc)
    {
        g_dbOpen = false;
        MessageBox(nppData._nppHandle, TEXT("Cannot find or open FingerText.db3 in config folder"), TEXT("FingerText"), MB_ICONERROR);
    } else
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
    ::_tcscpy(g_fttempPath,path);
    //::_tcscpy(g_groupPath,path);
    ::_tcscat(g_iniPath,TEXT("\\FingerText.ini"));
    ::_tcscat(g_ftbPath,TEXT("\\SnippetEditor.ftb"));
    ::_tcscat(g_fttempPath,TEXT("\\FingerText.fttemp"));
    //::_tcscat(g_groupPath,TEXT("\\SnippetGroup.ini"));
    

    setupConfigFile();

    g_snippetCache = new SnipIndex [g_snippetListLength];
    if (PathFileExists(g_ftbPath) == FALSE) emptyFile(g_ftbPath);
    if (PathFileExists(g_fttempPath) == FALSE) emptyFile(g_fttempPath);

    //if (PathFileExists(g_groupPath) == FALSE) writeDefaultGroupFile(); 
}

void emptyFile(TCHAR* fileName)
{
    //	if (PathFileExists(g_consolePath) == FALSE) emptyFile(g_consolePath);
    //if (PathFileExists(g_switcherPath) == FALSE) emptyFile(g_switcherPath);
    std::ofstream File;
    File.open(fileName,std::ios::out|std::ios::trunc);
    File.close();
}

void resetDefaultSettings()
{   
    g_snippetListLength = DEFAULT_SNIPPET_LIST_LENGTH;
    g_snippetListOrderTagType = DEFAULT_SNIPPET_LIST_ORDER_TAG_TYPE;
    g_tabTagCompletion = DEFAULT_TAB_TAG_COMPLETION;
    g_liveHintUpdate = DEFAULT_LIVE_HINT_UPDATE;
    g_indentReference = DEFAULT_INDENT_REFERENCE;
    g_chainLimit = DEFAULT_CHAIN_LIMIT;
    g_preserveSteps = DEFAULT_PRESERVE_STEPS;
    //g_escapeChar = DEFAULT_ESCAPE_CHAR;
    //g_importOverWriteOption = DEFAULT_IMPORT_OVERWRITE_OPTION;
    g_importOverWriteConfirm = DEFAULT_IMPORT_OVERWRITE_CONFIRM;
    g_inclusiveTriggerTextCompletion = DEFAULT_INCLUSIVE_TRIGGERTEXT_COMPLETION;
    g_livePreviewBox = DEFAULT_LIVE_PREVIEW_BOX;

    g_customScope = DEFAULT_CUSTOM_SCOPE;
    g_customEscapeChar = DEFAULT_CUSTOM_ESCAPE_CHAR;
}

//void saveCustomScope()
//{
//    writeConfigTextChar(g_customScope,TEXT("custom_scope"));
//}

void writeConfig()
{
    writeConfigText(g_snippetListLength,TEXT("snippet_list_length"));
    writeConfigText(g_snippetListOrderTagType,TEXT("snippet_list_order_tagtype"));
    writeConfigText(g_tabTagCompletion,TEXT("tab_tag_completion"));
    writeConfigText(g_liveHintUpdate,TEXT("live_hint_update"));
    writeConfigText(g_indentReference,TEXT("indent_reference"));
    writeConfigText(g_chainLimit,TEXT("chain_limit"));
    writeConfigText(g_preserveSteps,TEXT("preserve_steps"));
    //writeConfigText(g_escapeChar,TEXT("escape_char_level"));
    //writeConfigText(g_importOverWriteOption,TEXT("import_overwrite_option"));
    writeConfigText(g_importOverWriteConfirm,TEXT("import_overwrite_confirm"));
    writeConfigText(g_inclusiveTriggerTextCompletion,TEXT("inclusive_triggertext_completion"));
    writeConfigText(g_livePreviewBox,TEXT("live_preview_box"));
    
    writeConfigTextChar(g_customEscapeChar,TEXT("escape_char"));
    writeConfigTextChar(g_customScope,TEXT("custom_scope"));
}

void loadConfig()
{
    g_snippetListLength = GetPrivateProfileInt(TEXT("FingerText"), TEXT("snippet_list_length"), DEFAULT_SNIPPET_LIST_LENGTH, g_iniPath);
    g_snippetListOrderTagType = GetPrivateProfileInt(TEXT("FingerText"), TEXT("snippet_list_order_tagtype"), DEFAULT_SNIPPET_LIST_ORDER_TAG_TYPE, g_iniPath);
    g_tabTagCompletion = GetPrivateProfileInt(TEXT("FingerText"), TEXT("tab_tag_completion"), DEFAULT_TAB_TAG_COMPLETION, g_iniPath);
    g_liveHintUpdate = GetPrivateProfileInt(TEXT("FingerText"), TEXT("live_hint_update"), DEFAULT_LIVE_HINT_UPDATE, g_iniPath);
    g_indentReference = GetPrivateProfileInt(TEXT("FingerText"), TEXT("indent_reference"), DEFAULT_INDENT_REFERENCE, g_iniPath);
    g_chainLimit = GetPrivateProfileInt(TEXT("FingerText"), TEXT("chain_limit"), DEFAULT_CHAIN_LIMIT, g_iniPath);
    g_preserveSteps = GetPrivateProfileInt(TEXT("FingerText"), TEXT("preserve_steps"), DEFAULT_PRESERVE_STEPS, g_iniPath);
    //g_escapeChar = GetPrivateProfileInt(TEXT("FingerText"), TEXT("escape_char_level"), DEFAULT_ESCAPE_CHAR, g_iniPath);
    //g_importOverWriteOption = GetPrivateProfileInt(TEXT("FingerText"), TEXT("import_overwrite_option"), DEFAULT_IMPORT_OVERWRITE_OPTION, g_iniPath);
    g_importOverWriteConfirm = GetPrivateProfileInt(TEXT("FingerText"), TEXT("import_overwrite_confirm"), DEFAULT_IMPORT_OVERWRITE_CONFIRM, g_iniPath);
    g_inclusiveTriggerTextCompletion = GetPrivateProfileInt(TEXT("FingerText"), TEXT("inclusive_triggertext_completion"), DEFAULT_INCLUSIVE_TRIGGERTEXT_COMPLETION, g_iniPath);
    g_livePreviewBox = GetPrivateProfileInt(TEXT("FingerText"), TEXT("live_preview_box"), DEFAULT_LIVE_PREVIEW_BOX, g_iniPath);

    GetPrivateProfileString(TEXT("FingerText"), TEXT("escape_char"),DEFAULT_CUSTOM_ESCAPE_CHAR,g_customEscapeChar,MAX_PATH,g_iniPath);
    GetPrivateProfileString(TEXT("FingerText"), TEXT("custom_scope"),DEFAULT_CUSTOM_SCOPE,g_customScope,MAX_PATH,g_iniPath);
}

void setupConfigFile()
{
    //TODO: lazy loading of config.....
    g_version = ::GetPrivateProfileInt(TEXT("FingerText"), TEXT("version"), 0, g_iniPath);
    
    if (g_version == VERSION_LINEAR)  // current version
    {
        loadConfig();
        writeConfig();
        g_newUpdate = false;
        
    } else if ((g_version >= VERSION_KEEP_CONFIG_START) && (g_version <= VERSION_KEEP_CONFIG_END))// for version changes that do not want to reset database
    {
        g_version = VERSION_LINEAR;
        writeConfigText(g_version,TEXT("version"));
                
        loadConfig(); 
        writeConfig(); // TODO: think about method to get rid of the need to write config every time we load npp
        g_newUpdate = true;
    } else // for version that need database reset
    {
        g_version = VERSION_LINEAR;
        writeConfigText(g_version,TEXT("version"));
        resetDefaultSettings();
        
        writeConfig();
        //saveCustomScope();
        g_newUpdate = true;
    }
}


void writeConfigText(int configInt, TCHAR* section)
{
    wchar_t configText[32];
    _itow_s(configInt,configText, 10,10);
    ::WritePrivateProfileString(TEXT("FingerText"), section, configText, g_iniPath);
}

void writeConfigTextChar(TCHAR* configChar, TCHAR* section)
{
    ::WritePrivateProfileString(TEXT("FingerText"), section, configChar, g_iniPath);
}


//void writeDefaultGroupFile()
//{
//    ::WritePrivateProfileString(TEXT("Snippet Group"), TEXT("LANG_0"), TEXT(".txt|.ini|.log"), g_groupPath);
//    ::WritePrivateProfileString(TEXT("Snippet Group"), TEXT("LANG_1"), TEXT(".php"), g_groupPath);
//    ::WritePrivateProfileString(TEXT("Snippet Group"), TEXT("LANG_2"), TEXT(".c|.h"), g_groupPath);
//    ::WritePrivateProfileString(TEXT("Snippet Group"), TEXT("LANG_3"), TEXT(".cpp|.hpp"), g_groupPath);
//
//    //::WritePrivateProfileString(TEXT("Snippet Group"), TEXT("GROUP.rb"), TEXT(".rb"), g_groupPath);
// 
//}


int getCurrentTag(HWND curScintilla, int posCurrent, char **buffer, int triggerLength)
{
	int length = -1;

    int posBeforeTag;
    if (triggerLength<=0)
    {
        //TODO: global variable for word Char?
        char wordChar[MAX_PATH]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";

        //alertNumber(wcslen(g_customEscapeChar));
        //TODO: potential performance improvement by forming the wordchar with escape char that the initialization
        if (wcslen(g_customEscapeChar)>0)
        {
            char *customEscapeChar = NULL;
            customEscapeChar = new char[MAX_PATH];
            convertToUTF8(g_customEscapeChar, &customEscapeChar);
            strcat(wordChar,customEscapeChar);
            delete [] customEscapeChar;
        }

        //if (g_escapeChar == 1)
        //{
        //    strcat(wordChar,"<");
        //}

        ::SendMessage(curScintilla,	SCI_SETWORDCHARS, 0, (LPARAM)wordChar);
	    posBeforeTag = static_cast<int>(::SendMessage(curScintilla,	SCI_WORDSTARTPOSITION, posCurrent, 1));
        ::SendMessage(curScintilla,	SCI_SETCHARSDEFAULT, 0, 0);
    } else
    {
        posBeforeTag = posCurrent - triggerLength;
    }
                
    if (posCurrent - posBeforeTag < 100) // Max tag length 100
    {
        *buffer = new char[(posCurrent - posBeforeTag) + 1];
		Sci_TextRange tagRange;
		tagRange.chrg.cpMin = posBeforeTag;
		tagRange.chrg.cpMax = posCurrent;
		tagRange.lpstrText = *buffer;

	    ::SendMessage(curScintilla, SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tagRange));
		length = (posCurrent - posBeforeTag);
	}
    
	return length;
}

void convertToUTF8(TCHAR *orig, char **utf8)
{
    if (orig == NULL)
    {
        *utf8 = NULL;
    } else
    {
	    int multibyteLength = WideCharToMultiByte(CP_UTF8, 0, orig, -1, NULL, 0, 0, 0);
	    *utf8 = new char[multibyteLength + 1];
	    WideCharToMultiByte(CP_UTF8, 0, orig, -1, *utf8, multibyteLength, 0, 0);
    }
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
		data.dlgID = SNIPPET_DOCK_INDEX;
		::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
        snippetDock.display();
	} else
    {
        snippetDock.display(!snippetDock.isVisible());
    }
    snippetHintUpdate();
}


void snippetHintUpdate()
{     
    if ((!g_editorView) && (g_liveHintUpdate==1))
    {
        if (snippetDock.isVisible())
        {
            g_liveHintUpdate=0;
            HWND curScintilla = getCurrentScintilla();
            if ((::SendMessage(curScintilla,SCI_GETMODIFY,0,0)!=0) && (::SendMessage(curScintilla,SCI_SELECTIONISRECTANGLE,0,0)==0))
            {
                int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                char *partialTag;
	            int tagLength = getCurrentTag(curScintilla, posCurrent, &partialTag);
                
                if (tagLength==0)
                {
                    updateDockItems(false,false);
                } else if ((tagLength>0) && (tagLength<20))
                {
                    //alertNumber(tagLength);
                    char similarTag[MAX_PATH]="";
                    if (g_inclusiveTriggerTextCompletion==1) strcat(similarTag,"%");
                    strcat(similarTag,partialTag);
                    strcat(similarTag,"%");
            
                    updateDockItems(false,false,similarTag);
                }
                
                if (tagLength>=0) delete [] partialTag;   
            }
            g_liveHintUpdate=1;
        }
    }
    if (g_modifyResponse) refreshAnnotation();
}

void updateDockItems(bool withContent, bool withAll, char* tag)
{
    
    g_liveHintUpdate--;

    int scopeLength=0;
    int triggerLength=0;
    int contentLength=0;
    int tempScopeLength=0;
    int tempTriggerLength=0;
    int tempContentLength=0;

    //g_snippetCacheSize=snippetDock.getLength();
    
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
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets ORDER BY tagType DESC,tag DESC LIMIT ? ", -1, &stmt, NULL);
        } else 
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets WHERE (tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ?) AND tag LIKE ? ORDER BY tagType DESC,tag DESC LIMIT ? ", -1, &stmt, NULL);
        }
    } else
    {
        if (withAll)
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets ORDER BY tag DESC,tagType DESC LIMIT ? ", -1, &stmt, NULL);
        } else 
        {
            sqlitePrepare = sqlite3_prepare_v2(g_db, "SELECT tag,tagType,snippet FROM snippets WHERE (tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ?) AND tag LIKE ? ORDER BY tag DESC,tagType DESC LIMIT ? ", -1, &stmt, NULL);
        }
    }
    
    
	if (g_dbOpen && SQLITE_OK == sqlitePrepare)
	{
        char *customScope = NULL;
        customScope = new char[MAX_PATH];
        
        char *tagType1 = NULL;
        TCHAR *fileType1 = NULL;
		fileType1 = new TCHAR[MAX_PATH];
        char *tagType2 = NULL;
        TCHAR *fileType2 = NULL;
		fileType2 = new TCHAR[MAX_PATH];

        if (withAll)
        {
            char snippetCacheSizeText[10];
            ::_itoa(g_snippetListLength, snippetCacheSizeText, 10); 
            sqlite3_bind_text(stmt, 1, snippetCacheSizeText, -1, SQLITE_STATIC);
        } else
        {   
            convertToUTF8(g_customScope, &customScope);
            sqlite3_bind_text(stmt, 1, customScope, -1, SQLITE_STATIC);
            
            ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType1);
            convertToUTF8(fileType1, &tagType1);
            sqlite3_bind_text(stmt, 2, tagType1, -1, SQLITE_STATIC);

            ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)fileType2);
            convertToUTF8(fileType2, &tagType2);
            sqlite3_bind_text(stmt, 3, tagType2, -1, SQLITE_STATIC);

            sqlite3_bind_text(stmt, 4, getLangTagType(), -1, SQLITE_STATIC);
        
            sqlite3_bind_text(stmt, 5, "GLOBAL", -1, SQLITE_STATIC);

            sqlite3_bind_text(stmt, 6, tag, -1, SQLITE_STATIC);

            //TODO: potential performance improvement by just setting 100
            char snippetCacheSizeText[10];
            ::_itoa(g_snippetListLength, snippetCacheSizeText, 10); 
            sqlite3_bind_text(stmt, 7, snippetCacheSizeText, -1, SQLITE_STATIC);
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

        delete [] customScope;
        delete [] tagType1;
        delete [] fileType1;
        delete [] tagType2;
        delete [] fileType2;
    }
    sqlite3_finalize(stmt);
    populateDockItems();
    
    if (g_livePreviewBox==1) showPreview(true);

    g_liveHintUpdate++;
    ::SendMessage(getCurrentScintilla(),SCI_GRABFOCUS,0,0);   
    
}

void populateDockItems()
{
    for (int j=0;j<g_snippetListLength;j++)
    {
        if (g_snippetCache[j].scope !=NULL)
        {
            char newText[300]="";
            
            int triggerTextLength = strlen(g_snippetCache[j].triggerText);
            
            //TODO: option to show trigger text first

            //if (strcmp(g_snippetCache[j].scope,"GLOBAL")==0)
            //{
            //    strcat(newText,"<<");
            //    strcat(newText,g_snippetCache[j].scope);
            //    strcat(newText,">>");
            //} else
            //{

            strcat(newText,"<");
            strcat(newText,g_snippetCache[j].scope);
            strcat(newText,">");
            //}
            
            //TODO: make this 14 customizable in settings
            int scopeLength = 14 - strlen(newText);
            if (scopeLength < 3) scopeLength = 3;
            for (int i=0;i<scopeLength;i++)
            {
                strcat(newText," ");
            }
            strcat(newText,g_snippetCache[j].triggerText);

            //size_t origsize = strlen(newText) + 1;
            //const size_t newsize = 400;
            //size_t convertedChars = 0;
            //wchar_t convertedTagText[newsize];
            //mbstowcs_s(&convertedChars, convertedTagText, origsize, newText, _TRUNCATE);

            wchar_t* convertedTagText;
            convertToWideChar(newText,&convertedTagText);

            snippetDock.addDockItem(convertedTagText);
            delete [] convertedTagText;
        }
    }
}

void clearCache()
{   
    //TODO: fix update dockitems memoryleak
    //g_snippetCacheSize=g_snippetListLength;
        
    for (int i=0;i<g_snippetListLength;i++)
    {
        g_snippetCache[i].triggerText=NULL;
        g_snippetCache[i].scope=NULL;
        g_snippetCache[i].content=NULL;
    }
}

void exportAndClearSnippets()
{
    //TODO: move the snippet export counting message out of the export snippets function so that it can be shown together with the clear snippet message
    if (exportSnippets())
    {
        int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("Are you sure that you want to clear the whole snippet database?"), TEXT("FingerText"), MB_YESNO);
        if (messageReturn == IDYES)
        {
            clearAllSnippets();
            ::MessageBox(nppData._nppHandle, TEXT("All snippets are deleted."), TEXT("FingerText"), MB_OK);
        } else 
        {
            ::MessageBox(nppData._nppHandle, TEXT("Snippet clearing is aborted."), TEXT("FingerText"), MB_OK);
        }
    }
}

void exportSnippetsOnly()
{
    exportSnippets();
}

void clearAllSnippets()
{
    sqlite3_stmt *stmt;
                
    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets", -1, &stmt, NULL))
    {
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "VACUUM", -1, &stmt, NULL))
    {
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    updateDockItems(false,false);
}

bool exportSnippets()
{
    //TODO: Can actually add some informtiaon at the end of the exported snippets......can be useful information like version number or just describing the package

    g_liveHintUpdate--;  // Temporary turn off live update as it disturb exporting

    bool success = false;

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

        g_snippetListLength = 100000;
        g_snippetCache = new SnipIndex [g_snippetListLength];
        updateDockItems(true,true,"%");
        
        int exportCount = 0;
        for (int j=0;j<g_snippetListLength;j++)
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
        success = true;

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
            wcscat(exportCountText,TEXT("No snippets are exported."));
        }
        
        ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
        ::MessageBox(nppData._nppHandle, exportCountText, TEXT("FingerText"), MB_OK);
    }

    g_snippetListLength = GetPrivateProfileInt(TEXT("FingerText"), TEXT("snippet_list_length"), DEFAULT_SNIPPET_LIST_LENGTH, g_iniPath);
    g_snippetCache = new SnipIndex [g_snippetListLength];
    updateDockItems(true,true,"%");
    g_liveHintUpdate++;

    return success;
    
}

char* cleanupString(char *str, char key)
{
    if (str==NULL) return NULL;

    char *from, *to;
    from=to=str;

    while ((*from != key) && (*to++=*from),*from++);
    return str;
}

void cleanupString2(char* str, char key)
{
    char *from, *to;
    from=to=str;

    while ((*from != key) && (*to++=*from),*from++);
    //return str;
}


void convertToWideChar(char* orig, wchar_t **wideChar)
{
    if (orig == NULL)
    {
        *wideChar = NULL;
    } else
    {
        size_t origsize = strlen(orig) + 1;
        size_t convertedChars = 0;
        *wideChar = new wchar_t[origsize*4+1];
        mbstowcs_s(&convertedChars, *wideChar, origsize, orig, _TRUNCATE);
    }
}

//TODO: importsnippet and savesnippets need refactoring sooooo badly
//TODO: Or it should be rewrite, import snippet should open the snippetediting.ftb, turn or annotation, and cut and paste the snippet on to that file and use the saveSnippet function
void importSnippets()
{
    //TODO: importing snippet will change the current directory, which is not desirable effect
    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
    {
        //TODO: prompt for closing tab instead of just warning
        ::MessageBox(nppData._nppHandle, TEXT("Please close all the snippet editing tabs (SnippetEditor.ftb) before importing any snippet pack."), TEXT("FingerText"), MB_OK);
        return;
    }

    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_fttempPath))
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_SAVE);
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
    }   

    g_liveHintUpdate--;
    
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
        //int conflictOverwrite = IDNO;
        //if (g_importOverWriteOption==1)
        //{
        //   conflictOverwrite = ::MessageBox(nppData._nppHandle, TEXT("Do you want to overwrite the database when the imported snippets has conflicts with existing snippets? Press Yes if you want to overwrite, No if you want to keep both versions."), TEXT("FingerText"), MB_YESNO);
        //}
        int conflictKeepCopy = IDNO;
        conflictKeepCopy = ::MessageBox(nppData._nppHandle, TEXT("Do you want to keep both versions if the imported snippets are conflicting with existing one?\r\n\r\nYes - Keep both versions\r\nNo - Overwrite existing version\r\nCancel - Stop importing"), TEXT("FingerText"), MB_YESNOCANCEL);

        if (conflictKeepCopy == IDCANCEL)
        {
            ::MessageBox(nppData._nppHandle, TEXT("Snippet importing aborted."), TEXT("FingerText"), MB_OK);
            return;
        }

        //::MessageBox(nppData._nppHandle, (LPCWSTR)fileName, TEXT("Trace"), MB_OK);
        std::ifstream file;
        file.open((LPCWSTR)fileName);   // TODO: This part may cause problem in chinese file names

        file.seekg(0, std::ios::end);
        int fileLength = file.tellg();
        file.seekg(0, std::ios::beg);

        if (file.is_open())
        {
            char* fileText = new char[fileLength+1];
            ZeroMemory(fileText,fileLength);

            file.read(fileText,fileLength);
            file.close();
        
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            int importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
            ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);
        
            HWND curScintilla = getCurrentScintilla();
            //::SendMessage(curScintilla, SCI_SETCODEPAGE,65001,0);
            ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)fileText);
            ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
            ::SendMessage(curScintilla, SCI_NEWLINE, 0, 0);

            delete [] fileText;
        
            int importCount=0;
            int conflictCount=0;
            int next=0;
            char* snippetText;
            char* tagText; 
            char* tagTypeText;
            int snippetPosStart;
            int snippetPosEnd;
            bool notOverWrite;
            char* snippetTextOld;
            //char* snippetTextOldCleaned;
            do
            {
                //import snippet do not have the problem of " " in save snippet because of the space in  "!$[FingerTextData FingerTextData]@#"
                ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
                                
                getLineChecked(&tagText,curScintilla,1,TEXT("Error: Invalid TriggerText. The ftd file may be corrupted."));
                getLineChecked(&tagTypeText,curScintilla,2,TEXT("Error: Invalid Scope. The ftd file may be corrupted."));
                
                // Getting text after the 3rd line until the tag !$[FingerTextData FingerTextData]@#
                ::SendMessage(curScintilla,SCI_GOTOLINE,3,0);
                snippetPosStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                //int snippetPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
            
                searchNext(curScintilla, "!$[FingerTextData FingerTextData]@#");
                snippetPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
                ::SendMessage(curScintilla,SCI_SETSELECTION,snippetPosStart,snippetPosEnd);
            
                snippetText = new char[snippetPosEnd-snippetPosStart + 1];
                ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(snippetText));
            
                ::SendMessage(curScintilla,SCI_SETSELECTION,0,snippetPosEnd+1); // This +1 corrupt the ! in !$[FingerTextData FingerTextData]@# so that the program know a snippet is finished importing
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");

                sqlite3_stmt *stmt;
                
                notOverWrite = false;
            
                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                {
                    sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                    if(SQLITE_ROW == sqlite3_step(stmt))
                    {
                        const char* extracted = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                        
                        snippetTextOld = new char[strlen(extracted)+1];
                        ZeroMemory(snippetTextOld,sizeof(snippetTextOld));
                        strcpy(snippetTextOld, extracted);
                        //
                        //snippetTextOldCleaned = new char[strlen(snippetTextOld)];
                        //ZeroMemory(snippetTextOldCleaned,sizeof(snippetTextOldCleaned));
                        
                        //snippetTextOldCleaned = cleanupString(snippetTextOld,'\r');
                        snippetTextOld = cleanupString(snippetTextOld,'\r');

                        //if (strlen(snippetTextNew) == strlen(snippetText)) alert();
                        //if (strcmp(snippetText,snippetTextOldCleaned) == 0)
                        if (strcmp(snippetText,snippetTextOld) == 0)
                        {
                            delete [] snippetTextOld;
                            notOverWrite = true;
                            //sqlite3_finalize(stmt);
                        } else
                        {
                            delete [] snippetTextOld;
                        //    sqlite3_finalize(stmt);
                            if (conflictKeepCopy==IDNO)
                            {
                                if (g_importOverWriteConfirm == 1)
                                {
                                    // TODO: may be moving the message to earlier location so that the text editor will be showing the message that is about to be overwriting into the database
                                    // TODO: try showing the conflict message on the editor
                        
                                    ::SendMessage(curScintilla,SCI_GOTOLINE,0,0);
                                    //TODO: refactor this repeated replacesel action
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
                                notOverWrite = true;
                                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
                                {
                                    importCount++;
                                    //TODO: add file name to the stamp
                                    char* dateText = getDateTime("yyyyMMdd");
                                    char* timeText = getDateTime("HHmmss",false);
                                    char* tagTextsuffixed;
                                    tagTextsuffixed = new char [strlen(tagText)+256];
                                
                                    strcpy(tagTextsuffixed,tagText);
                                    strcat(tagTextsuffixed,".Conflict");
                                    strcat(tagTextsuffixed,dateText);
                                    strcat(tagTextsuffixed,timeText);
                                    
                                    sqlite3_bind_text(stmt, 1, tagTextsuffixed, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
                                
                                    sqlite3_step(stmt);
                                    //sqlite3_finalize(stmt);
                                    conflictCount++;
                                    delete [] tagTextsuffixed;
                                    delete [] dateText;
                                    delete [] timeText;
                                }
                            }
                        }
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
                //delete [] tagText;
                //delete [] tagTypeText;
                //delete [] snippetText;
            
                ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
                updateDockItems(false,false);
            
                ::SendMessage(curScintilla,SCI_GOTOPOS,0,0);
                next = searchNext(curScintilla, "!$[FingerTextData FingerTextData]@#");
            } while (next>=0);
            
            wchar_t importCountText[200] = TEXT("");
            
            if (importCount>1)
            {
                ::_itow_s(importCount, importCountText, 10, 10);
                wcscat(importCountText,TEXT(" snippets are imported."));
            } else if (importCount==1)
            {
                wcscat(importCountText,TEXT("1 snippet is imported."));
            } else
            {
                wcscat(importCountText,TEXT("No Snippets are imported."));
            }

            if (conflictCount>0)
            {
                //TODO: more detail messages and count the number of conflict or problematic snippets
                wcscat(importCountText,TEXT("\r\n\r\nThere are some conflicts between the imported and existing snippets. You may go to the snippet editor to clean them up."));
            }
            //::MessageBox(nppData._nppHandle, TEXT("Complete importing snippets"), TEXT("FingerText"), MB_OK);
            ::MessageBox(nppData._nppHandle, importCountText, TEXT("FingerText"), MB_OK);
            
            ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);

                //updateMode();
                //updateDockItems();
        }
        //delete [] fileText;
    }
    g_liveHintUpdate++;

}

//void importSnippetsBackup()
//{
//    // TODO: close snippet editing window before import 
//    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
//    {
//        ::MessageBox(nppData._nppHandle, TEXT("Please close all the snippet editing tabs (SnippetEditor.ftb) before importing any snippet pack."), TEXT("FingerText"), MB_OK);
//        return;
//    }
//
//    g_liveHintUpdate--;
//    
//    OPENFILENAME ofn;
//    char fileName[MAX_PATH] = "";
//    ZeroMemory(&ofn, sizeof(ofn));
//
//    ofn.lStructSize = sizeof(OPENFILENAME);
//    ofn.hwndOwner = NULL;
//    ofn.lpstrFilter = TEXT("FingerText Datafiles (*.ftd)\0*.ftd\0");
//    ofn.lpstrFile = (LPWSTR)fileName;
//    ofn.nMaxFile = MAX_PATH;
//    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
//    ofn.lpstrDefExt = TEXT("");
//    
//    if (::GetOpenFileName(&ofn))
//    {
//        //::MessageBox(nppData._nppHandle, (LPCWSTR)fileName, TEXT("Trace"), MB_OK);
//        std::ifstream file;
//
//        file.open((LPCWSTR)fileName);   // This part may cause problem in chinese file names
//
//        file.seekg(0, std::ios::end);
//        int fileLength = file.tellg();
//        file.seekg(0, std::ios::beg);
//
//        //wchar_t countText[10];
//        //::_itow_s(fileLength, countText, 10, 10); 
//        //::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);
//        
//        char* fileText = new char[fileLength+1];
//        ZeroMemory(fileText,strlen(fileText));
//        if (file.is_open())
//        {
//            file.read(fileText,fileLength);
//            file.close();
//        
//            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
//            int importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
//            ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);
//
//
//
//            HWND curScintilla = getCurrentScintilla();
//
//            int conflictOverwrite = IDNO;
//            if (g_importOverWriteOption==1)
//            {
//               conflictOverwrite = ::MessageBox(nppData._nppHandle, TEXT("Do you want to overwrite the database when the imported snippets has conflicts with existing snippets? Press Yes if you want to overwrite, No if you want to keep both versions."), TEXT("FingerText"), MB_YESNO);
//            }
//
//            
//
//            //::SendMessage(curScintilla, SCI_SETCODEPAGE,65001,0);
//            ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)fileText);
//            ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
//            ::SendMessage(curScintilla, SCI_NEWLINE, 0, 0);
//            int importCount=0;
//            int conflictCount=0;
//            int next=0;
//            char* snippetText;
//            char* tagText; 
//            char* tagTypeText;
//            int snippetPosStart;
//            int snippetPosEnd;
//            bool notOverWrite;
//            char* snippetTextOld;
//            char* snippetTextOldCleaned;
//            do
//            {
//                //import snippet do not have the problem of " " in save snippet because of the space in  "!$[FingerTextData FingerTextData]@#"
//                ::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
//                
//                
//                
//                getLineChecked(&tagText,curScintilla,1,TEXT("Error: Invalid TriggerText. The ftd file may be corrupted."));
//                getLineChecked(&tagTypeText,curScintilla,2,TEXT("Error: Invalid Scope. The ftd file may be corrupted."));
//                
//                // Getting text after the 3rd line until the tag !$[FingerTextData FingerTextData]@#
//                ::SendMessage(curScintilla,SCI_GOTOLINE,3,0);
//                snippetPosStart = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
//                //int snippetPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
//
//                searchNext(curScintilla, "!$[FingerTextData FingerTextData]@#");
//                snippetPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
//                ::SendMessage(curScintilla,SCI_SETSELECTION,snippetPosStart,snippetPosEnd);
//            
//                snippetText = new char[snippetPosEnd-snippetPosStart + 1];
//                ::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(snippetText));
//            
//                ::SendMessage(curScintilla,SCI_SETSELECTION,0,snippetPosEnd+1); // This +1 corrupt the ! in !$[FingerTextData FingerTextData]@# so that the program know a snippet is finished importing
//                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
//                //::SendMessage(curScintilla,SCI_GOTOLINE,1,0);
//                //::SendMessage(curScintilla,SCI_SETSELECTION,0,::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
//                //::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
//
//                sqlite3_stmt *stmt;
//                
//            
//                notOverWrite = false;
//
//                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
//                {
//                    sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
//                    sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
//                    if(SQLITE_ROW == sqlite3_step(stmt))
//                    {
//                        const char* extracted = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
//                        snippetTextOld = new char[strlen(extracted)];
//                        memset(snippetTextOld,0,strlen(snippetTextOld));
//                        strcat(snippetTextOld, extracted);
//
//                        snippetTextOldCleaned = new char[strlen(snippetTextOld)];
//                        memset(snippetTextOldCleaned,0,strlen(snippetTextOldCleaned));
//
//
//                        //char* snippetTextOldConverted; 
//                        //snippetTextOldConverted = new char[strlen(extracted)];
//                        //snippetTextOldConverted = convertEol(snippetTextOld);
//                        
//                        snippetTextOldCleaned = cleanupString(snippetTextOld);
//                 
//                        
//                        //if (strlen(snippetTextNew) == strlen(snippetText)) alert();
//
//                        //if (strncmp(snippetText,snippetTextNew,3) == 0)
//                        if (strcmp(snippetText,snippetTextOldCleaned) == 0)
//                        {
//
//                            notOverWrite = true;
//                            sqlite3_finalize(stmt);
//
//                            
//
//                        } else
//                        {
//                            //alertNumber(strlen(snippetText));
//                            //alertNumber(strlen(snippetTextNew));
//                            //
//                            //alertCharArray(snippetText);
//                            //alertCharArray(snippetTextNew);
//
//                            sqlite3_finalize(stmt);
//
//                            if (conflictOverwrite==IDYES)
//                            {
//                                if (g_importOverWriteConfirm == 1)
//                                {
//
//
//                                    // TODO: may be moving the message to earlier location so that the text editor will be showing the message that is about to be overwriting into the database
//                                    // TODO: try showing the conflict message on the editor
//
//                                    ::SendMessage(curScintilla,SCI_GOTOLINE,0,0);
//  
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\nConflicting Snippet: \r\n\r\n     ");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagText);
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"  <");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)tagTypeText);
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)">\r\n");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n   (More details of the conflicts will be shown in future releases)");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n----------------------------------------\r\n");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"---------- [ Pending Imports ] ---------\r\n");
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"----------------------------------------\r\n");
//
//                                    int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("A snippet already exists, overwrite?"), TEXT("FingerText"), MB_YESNO);
//                                    if (messageReturn==IDNO)
//                                    {
//                                        //delete [] tagText;
//                                        //delete [] tagTypeText;
//                                        //delete [] snippetText;
//                                        // not overwrite
//                                        //::MessageBox(nppData._nppHandle, TEXT("The Snippet is not saved."), TEXT("FingerText"), MB_OK);
//                                        notOverWrite = true;
//
//                                        
//            
//                                    } else
//                                    {
//                                        // delete existing entry
//                                        if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
//                                        {
//                                            sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
//                                            sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
//                                            sqlite3_step(stmt);
//                                        } else
//                                        {
//                                            ::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT("FingerText"), MB_OK);
//                                        }
//                    
//                                    }
//                                    ::SendMessage(curScintilla,SCI_GOTOLINE,17,0);
//                                    ::SendMessage(curScintilla,SCI_SETSELECTION,0,::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
//                                    ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)"");
//
//                                } else
//                                {
//                                    // delete existing entry
//                                    if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
//                                    {
//                                        sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
//                                        sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
//                                        sqlite3_step(stmt);
//                                    } else
//                                    {
//                                        ::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT("FingerText"), MB_OK);
//                                    }
//
//                                }
//
//
//
//
//                            } else
//                            {
//                                notOverWrite = true;
//                                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
//                                {
//                                    importCount++;
//                                    
//                                    char* tagTextsuffixed;
//                                    tagTextsuffixed = new char [strlen(tagText)+255];
//
//
//                                    //TODO: refactor the timestamp generation to a new function
//                                        TCHAR time[128];
//                                        TCHAR date[128];
//                                        SYSTEMTIME formatTime;
//	                                    ::GetLocalTime(&formatTime);
//                                        //::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT + TIME_NOTIMEMARKER, &formatTime, NULL, time, 128);
//                                        ::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &formatTime, TEXT("HHmmss"), time, 128);
//                                        ::GetDateFormat(LOCALE_USER_DEFAULT, 0, &formatTime, TEXT("yyyyMMdd"), date, 128);
//
//                                        //if (date)
//                                        //{
//                                        //    ::GetDateFormat(LOCALE_USER_DEFAULT, type, &formatTime, NULL, time, 128);
//                                        //} else
//                                        //{
//                                            
//                                        //}
//                                        char timeText[MAX_PATH];
//	                                    WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, time, -1, timeText, MAX_PATH, NULL, NULL);
//                                        char dateText[MAX_PATH];
//	                                    WideCharToMultiByte((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, date, -1, dateText, MAX_PATH, NULL, NULL);
//
//                                    strcpy(tagTextsuffixed,tagText);
//                                    strcat(tagTextsuffixed,".Conflict");
//                                    strcat(tagTextsuffixed,dateText);
//                                    strcat(tagTextsuffixed,timeText);
//                                    
//                                    
//                                    sqlite3_bind_text(stmt, 1, tagTextsuffixed, -1, SQLITE_STATIC);
//                                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
//                                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
//            
//                                    
//                                    sqlite3_step(stmt);
//                                    conflictCount++;
//                                    delete [] tagTextsuffixed;
//                                    
//                                }
//                                sqlite3_finalize(stmt);
//
//
//                            }
//
//
//
//
//                        }
//                        
//                        
//                        
//
//                        
//                        
//                    } else
//                    {
//                        sqlite3_finalize(stmt);
//                    }
//                }
//            
//                if (notOverWrite == false && g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
//                {
//                    
//                    importCount++;
//                    
//                    // Then bind the two ? parameters in the SQLite SQL to the real parameter values
//                    sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
//                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
//                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
//            
//                    // Run the query with sqlite3_step
//                    sqlite3_step(stmt); // SQLITE_ROW 100 sqlite3_step() has another row ready
//                    //::MessageBox(nppData._nppHandle, TEXT("The Snippet is saved."), TEXT("FingerText"), MB_OK);
//                }
//                sqlite3_finalize(stmt);
//                //delete [] tagText;
//                //delete [] tagTypeText;
//                //delete [] snippetText;
//            
//                ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
//                updateDockItems(false,false);
//            
//                ::SendMessage(curScintilla,SCI_GOTOPOS,0,0);
//                next = searchNext(curScintilla, "!$[FingerTextData FingerTextData]@#");
//            } while (next>=0);
//        
//            wchar_t importCountText[200] = TEXT("");
//
//            if (importCount>1)
//            {
//                ::_itow_s(importCount, importCountText, 10, 10);
//                wcscat(importCountText,TEXT(" snippets are imported."));
//            } else if (importCount==1)
//            {
//                wcscat(importCountText,TEXT("1 snippet is imported."));
//            } else
//            {
//                wcscat(importCountText,TEXT("No Snippets are imported."));
//            }
//
//            //wchar_t conflictCountText[35] = TEXT("");
//
//            if (conflictCount>0)
//            {
//                //::_itow_s(conflictCount, conflictCountText, 10, 10);
//
//                wcscat(importCountText,TEXT("\r\n\r\nThere are some conflicts between the imported and existing snippets. You may go to the snippet editor to clean them up."));
//                
//            }
//
//            
//            //::MessageBox(nppData._nppHandle, TEXT("Complete importing snippets"), TEXT("FingerText"), MB_OK);
//            ::MessageBox(nppData._nppHandle, importCountText, TEXT("FingerText"), MB_OK);
//
//            ::SendMessage(curScintilla,SCI_SETSAVEPOINT,0,0);
//            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
//
//            //updateMode();
//            //updateDockItems();
//        }
//        delete [] fileText;
//    }
//    g_liveHintUpdate++;
//
//}
//

int promptSaveSnippet(TCHAR* message)
{
    int messageReturn = IDNO;
    if (g_editorView)
    {
        HWND curScintilla = ::getCurrentScintilla();

        if (message == NULL)
        {
            saveSnippet();
        } else if (::SendMessage(curScintilla,SCI_GETMODIFY,0,0)!=0)
        {
            messageReturn=::MessageBox(nppData._nppHandle, message, TEXT("FingerText"), MB_YESNO);
            if (messageReturn==IDYES)
            {
                saveSnippet();
            }
        }
    }
    return messageReturn;
}

void updateMode()
{
    //TODO: should change to edit mode and normal mode by a button, and dynamically adjust the dock content
    //HWND curScintilla = getCurrentScintilla();
    TCHAR fileType[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileType);
        
    if (::_tcscmp(fileType,TEXT("SnippetEditor.ftb"))==0)
    {
        snippetDock.toggleSave(true);
        g_editorView = true;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("EDIT MODE\r\n(Double click item in list to edit another snippet, Ctrl+S to save)"));
    } else if (g_enable)
    {
        snippetDock.toggleSave(false);
        g_editorView = false;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Enabled]\r\n(Type trigger text and hit tab to insert snippet)"));
    } else
    {
        snippetDock.toggleSave(false);
        g_editorView = false;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Disabled]\r\n(To enable: Plugins>FingerText>Toggle FingerText On/Off)"));
    }
}

void settings()
{
    // TODO: try putting settings into the ini files instead of just using annotation
    int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("Change the settings only when you know what you are doing. Messing up the ini can cause FingerText to stop working.\r\n\r\n Do you wish to continue?"), TEXT("FingerText"), MB_YESNO);
    if (messageReturn == IDYES)
    {
        if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_iniPath))
        {
            ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_iniPath);
        }
        HWND curScintilla = getCurrentScintilla();
        int lineCount = ::SendMessage(curScintilla, SCI_GETLINECOUNT, 0, 0)-1;
        ::SendMessage(curScintilla, SCI_ANNOTATIONCLEARALL, 0, 0);
        //TODO: move this part (and other text) to const char in another file.
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETTEXT, lineCount, (LPARAM)"\
 ; \r\n\
 ; This is the config file of FingerText.    \r\n\
 ; Do NOT mess up with the settings unless you know what you are doing \r\n\
 ; You need to restart Notepad++ to apply the changes\r\n\
 ; \r\n\
 ; \r\n\
 ; version                    --  Don't change this\r\n\
 ; snippet_list_order_tagtype --            0: The SnippetDock will order the snippets by trigger text\r\n\
 ;                                (default) 1: The SnippetDock will order the snippets by scope\r\n\
 ; indent_reference           --            0: The snippnet content will be inserted without any change in\r\n\
 ;                                             indentation\r\n\
 ;                                (default) 1: The snippnet content will be inserted at the same indentation\r\n\
 ;                                             level as the trigger text\r\n\
 ; chain_limit                --  This is the maximum number dynamic hotspots that can be triggered in one \r\n\
 ;                                snippet. Default is 20\r\n\
 ; snippet_list_length        --  The maximum number of items that can be displayed in the SnippetDock. Default\r\n\
 ;                                is 100 \r\n\
 ; tab_tag_completion         --  (default) 0: When a snippet is not found when the user hit [tab] key, FingerText\r\n\
 ;                                             will just send a tab\r\n\
 ;                                          1: When a snippet is not found when the user hit [tab] key, FingerText\r\n\
 ;                                             will try to find the closest match snippet name\r\n\
 ; live_hint_update           --            0: Turn off SnippetDock live update\r\n\
 ;                                (default) 1: Turn on SnippetDock live update\r\n\
 ; preserve_steps             --  Default is 0 and don't change it. It's for debugging purpose\r\n\
 ; escape_char_level          --  This entry is not in use anymore. Please use the 'escape_char' instead.\r\n\
 ; import_overwrite_confirm   --  (default) 0: A pop up confirmation message box everytime you overwrite a snippet.\r\n\
 ;                                          1: No confirmation message box when you overwrite a snippet.\r\n\
 ; import_overwrite_option    --  This entry is not in use anymore. \r\n\
 ; inclusive_triggertext_completion --            0: Tiggertext completion will only include triggertext which starts\r\n\
 ;                                                   with the characters you are typing.\r\n\
 ;                                      (default) 1: Tiggertext completion will only include triggertext which\r\n\
 ;                                                   includes the characters you are typing.\r\n\
 ; custom_scope               --  A user defined custom scope. For example if you put .rb here, you can use all the\r\n\
 ;                                .rb snippets in any files.\r\n\
 ; escape_char                --  Any text entered after this character will not be view as snippet. For example\r\n\
 ;                                if put <> here then you cannot trigger the snippet 'npp' by typing either '<npp'\r\n\
 ;                                or '>npp' and hit tab\r\n\
        ");
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETSTYLE, lineCount, STYLE_INDENTGUIDE);
        ::SendMessage(curScintilla, SCI_ANNOTATIONSETVISIBLE, lineCount, 0);
    }
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

//void keyUpdate()
//{
//    if (g_editorView)
//    {
//        refreshAnnotation();
//    }
//    //else
//    //{
//    //    snippetHintUpdate();
//    //}
//}

void refreshAnnotation()
{
    if (g_editorView)
    {
        g_modifyResponse = false;
        TCHAR fileType[MAX_PATH];
        //::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
        ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileType);
        
        if (::_tcscmp(fileType,TEXT("SnippetEditor.ftb"))==0)
        {
            HWND curScintilla = getCurrentScintilla();
            //::SendMessage(getCurrentScintilla(), SCI_ANNOTATIONCLEARALL, 0, 0);
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
                    # Only alphanumerics, underscores and periods are allowed.\r\n\r\n");
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
        g_modifyResponse = true;
    }
}

//For option dynamic hotspot
void cleanOptionItem()
{
    //g_optionArray[0] = "";
    //g_optionArray[1] = "";
    //g_optionArray[2] = "";
    //g_optionArray[3] = "";
    //g_optionArray[4] = "";
    int i = 0;
    while (i<g_optionNumber)
    {
        //alertNumber(i);
        delete [] g_optionArray[i];
        i++;
    };
    g_optionMode = false;
    g_optionNumber = 0;
    g_optionCurrent = 0;
}

void addOptionItem(char* item)
{
    if (g_optionNumber<20)
    {
        g_optionArray[g_optionNumber] = item;
        g_optionNumber++;
    }
}

//char* getOptionItem()
void updateOptionCurrent()
{
    //char* item;
    //int length = strlen(g_optionArray[g_optionCurrent]);
    //item = new char [length+1];
    //strcpy(item, g_optionArray[g_optionCurrent]);

    if (g_optionCurrent >= g_optionNumber-1) 
    {
        g_optionCurrent = 0;
    } else
    {
        g_optionCurrent++;
    }
    //return item;
    //return g_optionArray[g_optionCurrent];
}
void turnOffOptionMode()
{
    g_optionMode = false;
}

void optionNavigate(HWND &curScintilla)
{
    ::SendMessage(curScintilla,SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    updateOptionCurrent();
    ::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);
    ::SendMessage(curScintilla,SCI_GOTOPOS,g_optionStartPosition,0);
    g_optionEndPosition = g_optionStartPosition + strlen(g_optionArray[g_optionCurrent]);
    ::SendMessage(curScintilla,SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
}

void tagComplete()
{
    HWND curScintilla = getCurrentScintilla();
    int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
    if (triggerTag(posCurrent,true) > 0) snippetHintUpdate();
    //if (snippetComplete()) snippetHintUpdate();
}

//TODO: better triggertag, should allow for a list of scopes
bool triggerTag(int &posCurrent,bool triggerTextComplete, int triggerLength)
{
    HWND curScintilla = getCurrentScintilla();
    bool tagFound = false;
    char *tag;
	int tagLength = getCurrentTag(curScintilla, posCurrent, &tag, triggerLength);

    //int position = 0;
    //bool groupChecked = false;

    //int curLang = 0;
    //::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE ,0,(LPARAM)&curLang);
    //wchar_t curLangNumber[10];
    //wchar_t curLangText[20];
    //::wcscpy(curLangText, TEXT("LANG_"));
    //::_itow_s(curLang, curLangNumber, 10, 10);
    //::wcscat(curLangText, curLangNumber);

    if (tagLength > 0) //TODO: changing this to >0 fixed the problem of tag_tab_completion, but need to investigate more about the side effect
	{
        int posBeforeTag = posCurrent-tagLength;

        char *expanded = NULL;
        char *tagType = NULL;
        
        TCHAR *fileType = NULL;
        fileType = new TCHAR[MAX_PATH];

        // Check for custom scope
        convertToUTF8(g_customScope, &tagType);
        expanded = findTagSQLite(tag,tagType,triggerTextComplete); 
        
        // Check for snippets which matches ext part
        if (!expanded)
        {
            ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
            convertToUTF8(fileType, &tagType);
            expanded = findTagSQLite(tag,tagType,triggerTextComplete); 
            
            // Check for snippets which matches name part
            if (!expanded)
            {
                ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
                convertToUTF8(fileType, &tagType);
                expanded = findTagSQLite(tag,tagType,triggerTextComplete); 
                // Check for language specific snippets
                if (!expanded)
                {
                    expanded = findTagSQLite(tag,getLangTagType(),triggerTextComplete); 
                    // TODO: Hardcode the extension associated with each language type, check whether the extension are the same as the current extenstion, if not, use findtagSQLite to search for snippets using those scopes
                    
                    // Check for snippets which matches the current language group
                    //if (!expanded)
                    //{
                    //    groupChecked = true;
                    //    position = 0;
                    //    do
                    //    {   
                    //        tagType = getGroupScope(curLangText,position);
                    //        if (tagType)
                    //        {
                    //            expanded = findTagSQLite(tag,tagType,triggerTextComplete); 
                    //            
                    //        } else
                    //        {
                    //            break;
                    //        }
                    //        position++;
                    //    } while (!expanded);
                    //}

                    // Check for GLOBAL snippets
                    if (!expanded)
                    {
                        //groupChecked = false;
                        expanded = findTagSQLite(tag,"GLOBAL",triggerTextComplete); 

                    }
                }
            }
        }
        
        // Only if a tag is found in the above process, a replace tag or trigger text completion action will be done.
        if (expanded)
        {
            if (triggerTextComplete)
            {
                ::SendMessage(curScintilla,SCI_SETSEL,posBeforeTag,posCurrent);
                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)expanded);
                posBeforeTag = posBeforeTag+strlen(expanded);
            } else
            {
                replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
            }
                
		    tagFound = true;
        }


        //int level=1;
        //do
        //{
        //    expanded = findTagSQLite(tag,level,triggerTextComplete); 
        //    
		//	if (expanded)
        //    {
        //        if (triggerTextComplete)
        //        {
        //            ::SendMessage(curScintilla,SCI_SETSEL,posBeforeTag,posCurrent);
        //            ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)expanded);
        //            posBeforeTag = posBeforeTag+strlen(expanded);
        //        } else
        //        {
        //            replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
        //        }
        //        
		//		tagFound = true;
        //        break;
        //    } 
        //    level++;
        //} while (level<=3);
        delete [] fileType;
        //if (!groupChecked) delete [] tagType;
        delete [] tagType;
        delete [] expanded;
		delete [] tag;
        // return to the original path 
        // ::SetCurrentDirectory(curPath);

        // return to the original position 
        if (tagFound) ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
    } else if (tagLength == 0)
    {
        delete [] tag;
    }
    
    return tagFound;
}

//bool snippetComplete()
//{
//    HWND curScintilla = getCurrentScintilla();
//    int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
//    bool tagFound = false;
//    char *tag;
//	int tagLength = getCurrentTag(curScintilla, posCurrent, &tag);
//    int posBeforeTag=posCurrent-tagLength;
//    if (tagLength != 0)
//	{
//        char *expanded;
//
//        int level=1;
//        do
//        {
//            expanded = findTagSQLite(tag,level,true); 
//			if (expanded)
//            {
//                ::SendMessage(curScintilla,SCI_SETSEL,posBeforeTag,posCurrent);
//                ::SendMessage(curScintilla,SCI_REPLACESEL,0,(LPARAM)expanded);
//                posBeforeTag = posBeforeTag+strlen(expanded);
//                
//                //replaceTag(curScintilla, expanded, posCurrent, posBeforeTag);
//				tagFound = true;
//                break;
//            } 
//            level++;
//        } while (level<=3);
//
//        delete [] expanded;
//		delete [] tag;
//        // return to the original path 
//        // ::SetCurrentDirectory(curPath);
//    }
//    // return to the original position 
//    if (tagFound) ::SendMessage(curScintilla,SCI_GOTOPOS,posBeforeTag,0);
//    return tagFound;
//}

char* getLangTagType()
{
    int curLang = 0;
    ::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE ,0,(LPARAM)&curLang);
    //alertNumber(curLang);
    
    if ((curLang>54) || (curLang<0)) return "";

    //support the languages supported by npp 0.5.9, excluding "user defined language" abd "search results"
    char *s[] = {"Lang:TXT","Lang:PHP ","Lang:C","Lang:CPP","Lang:CS","Lang:OBJC","Lang:JAVA","Lang:RC",
                 "Lang:HTML","Lang:XML","Lang:MAKEFILE","Lang:PASCAL","Lang:BATCH","Lang:INI","Lang:NFO","",
                 "Lang:ASP","Lang:SQL","Lang:VB","Lang:JS","Lang:CSS","Lang:PERL","Lang:PYTHON","Lang:LUA",
                 "Lang:TEX","Lang:FORTRAN","Lang:BASH","Lang:FLASH","Lang:NSIS","Lang:TCL","Lang:LISP","Lang:SCHEME",
                 "Lang:ASM","Lang:DIFF","Lang:PROPS","Lang:PS","Lang:RUBY","Lang:SMALLTALK","Lang:VHDL","Lang:KIX",
                 "Lang:AU3","Lang:CAML","Lang:ADA","Lang:VERILOG","Lang:MATLAB","Lang:HASKELL","Lang:INNO","",
                 "Lang:CMAKE","Lang:YAML","Lang:COBOL","Lang:GUI4CLI","Lang:D","Lang:POWERSHELL","Lang:R"};
    
    return s[curLang];
    //return "";
}

void fingerText()
{

    //TODO: in general I should add logo to all the messages
    HWND curScintilla = getCurrentScintilla();

    if ((g_enable==false) || (::SendMessage(curScintilla,SCI_SELECTIONISRECTANGLE,0,0)==1))
    {
        ::SendMessage(curScintilla,SCI_TAB,0,0);
    } else
    {
        g_liveHintUpdate--;

        int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        
        //TODO: reexamine possible performance improvement
        bool optionTriggered = false;
        if (g_optionMode == true)
        {
            if (posCurrent == g_optionStartPosition)
            {
                optionNavigate(curScintilla);
                optionTriggered = true;
                g_optionMode = true; // TODO: investigate why this line is necessary
            } else
            {
                cleanOptionItem();
                g_optionMode = false;
            }
        }

        if (optionTriggered == false)
        {
            int posSelectionStart = ::SendMessage(curScintilla,SCI_GETSELECTIONSTART,0,0);
            int posSelectionEnd = ::SendMessage(curScintilla,SCI_GETSELECTIONEND,0,0);
            if (g_preserveSteps==0) ::SendMessage(curScintilla, SCI_BEGINUNDOACTION, 0, 0);
            bool tagFound = false;
            if (posSelectionStart==posSelectionEnd)
            {
                tagFound = triggerTag(posCurrent);
            }

            if (tagFound) ::SendMessage(curScintilla,SCI_AUTOCCANCEL,0,0);
            posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);

            bool navSpot = false;
            bool dynamicSpot = false;
            if (g_editorView == false)
            {
                //int specialSpot = searchNext(curScintilla, "$[![");
                //if (specialSpot>=0)
                //{
                    //::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent,0);
                
                    ////dynamic hotspot (chain snippet)
                    //chainSnippet(curScintilla, posCurrent);
                    //dynamicHotspot(curScintilla, posCurrent, 1);
                    ////dynamic hotspot (keyword spot)
                    //keyWordSpot(curScintilla, posCurrent);
                    //dynamicHotspot(curScintilla, posCurrent, 2);
                    ////dynamic hotspot (Command Line)
                    //executeCommand(curScintilla, posCurrent);
                    //dynamicHotspot(curScintilla, posCurrent, 3);

                    //dynamicHotspot(curScintilla, posCurrent);
                //}

                //if (searchNext(curScintilla, "$[![(")>=0)  
                dynamicSpot = dynamicHotspot(curScintilla, posCurrent); //TODO: May still consider do some checking before going into dynamic hotspot for performance improvement
                    
                if (g_preserveSteps==0) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
	            
                //if (specialSpot>=0)
                //{
                ::SendMessage(curScintilla,SCI_GOTOPOS,posCurrent,0);
                navSpot = hotSpotNavigation(curScintilla);

                if ((navSpot) || (dynamicSpot)) ::SendMessage(curScintilla,SCI_AUTOCCANCEL,0,0);
                //}
            } else
            {
                if (g_preserveSteps==0) ::SendMessage(curScintilla, SCI_ENDUNDOACTION, 0, 0);
            }

            bool completeFound = false;
            if (g_tabTagCompletion == 1)
            {
                if ((navSpot == false) && (tagFound == false) && (dynamicSpot==false)) 
	    	    {
                    ::SendMessage(curScintilla, SCI_GOTOPOS, posCurrent, 0);
                    //completeFound = snippetComplete();
                    completeFound = triggerTag(posCurrent,true);
                    if (completeFound)
                    {
                        ::SendMessage(curScintilla,SCI_AUTOCCANCEL,0,0);
                        snippetHintUpdate();
                    }
	    	    }
            }
            if ((navSpot == false) && (tagFound == false) && (completeFound==false) && (dynamicSpot==false)) 
            {
                restoreTab(curScintilla, posCurrent, posSelectionStart, posSelectionEnd);
            }
        } 
        g_liveHintUpdate++;
    }
}

//void Thread( void* pParams )
//{ 
//    system("npp -multiInst");
//    
//}

//void GenerateKey(int vk, BOOL bExtended) {
//
//    KEYBDINPUT  kb = {0};
//    INPUT       Input = {0};
//
//    /* Generate a "key down" */
//    if (bExtended) { kb.dwFlags  = KEYEVENTF_EXTENDEDKEY; }
//    kb.wVk  = vk;
//    Input.type  = INPUT_KEYBOARD;
//    Input.ki  = kb;
//    SendInput(1, &Input, sizeof(Input));
//
//    /* Generate a "key up" */
//    ZeroMemory(&kb, sizeof(KEYBDINPUT));
//    ZeroMemory(&Input, sizeof(INPUT));
//    kb.dwFlags  =  KEYEVENTF_KEYUP;
//    if (bExtended) { kb.dwFlags |= KEYEVENTF_EXTENDEDKEY; }
//    kb.wVk = vk;
//    Input.type = INPUT_KEYBOARD;
//    Input.ki = kb;
//    SendInput(1, &Input, sizeof(Input));
//
//    return;
//}


void testing()
{

    ::MessageBox(nppData._nppHandle, TEXT("Testing!"), TEXT("Trace"), MB_OK);
    
    HWND curScintilla = getCurrentScintilla();


    //Testing usage of cleanupstring()
    //char* test = new char[MAX_PATH];
    //strcpy(test,"To test the cleanupstring.");
    //alertCharArray(test);
    //test = cleanupString(test,'t');
    //alertCharArray(test);
    //delete [] test;
    //alertCharArray(test);

    //cleanOptionItem();
            
    //testing add and get optionitem, testing for memory leak
    //char* optionText;
    //optionText = new char[1000];
    //strcpy(optionText,"abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc");
    //addOptionItem(optionText);
    //char* optionText2;
    //optionText2 = new char[1000];
    //strcpy(optionText2,"defdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdefdef");
    //addOptionItem(optionText2);
    //char* optionText3;
    //optionText3 = new char[1000];
    //strcpy(optionText3,"ghighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighighi");
    //addOptionItem(optionText3);
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //cleanOptionItem();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //alertCharArray(g_optionArray[g_optionCurrent]);
    //updateOptionCurrent();
    //cleanOptionItem();

    //test using the getDateTime function
    //char* date = getDateTime("yyyyMMdd");
    //alertCharArray(date);
    //delete [] date;

    // testing to upper and to lower
    //char* str = new char[200];
    //strcpy(str,"aBcDe");
    //alertCharArray(str);
    //alertCharArray(::strupr(str));
    //alertCharArray(::strlwr(str));
    //delete [] str;


    //alertNumber(g_snippetListLength);
    //alertCharArray(getLangTagType());

    //char* testScope = NULL;
    //testScope = getGroupScope(TEXT("LANG_4"),0);
    //
    //alertCharArray(testScope);
    //testScope = getGroupScope(TEXT("LANG_4"),1);
    //alertCharArray(testScope);
    //testScope = getGroupScope(TEXT("LANG_4"),2);
    //alertCharArray(testScope);
    //testScope = getGroupScope(TEXT("LANG_4"),3);
    //if (testScope) alertCharArray(testScope);
    //if (testScope) delete [] testScope;

    //Test current language check
    //int curLang = 0;
    //::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE ,0,(LPARAM)&curLang);
    //
    //alertNumber(curLang);

    

    //g_customScope = TEXT(".cpp");
    //saveCustomScope();

    
    // Testing array of char array
    //char *s[] = {"Jan","Feb","Mar","April","May"};
    //char* a = "December";
    //s[1] = a;
    //alertCharArray(s[0]);
    //alertCharArray(s[1]);
    //alertCharArray(s[2]);
    //alertCharArray(s[3]);
    //
    //alertNumber(sizeof(s));
    //alertNumber(sizeof*(s));

    //char* b = "July";
    //s[1] = b;
    //alertCharArray(s[0]);
    //alertCharArray(s[1]);
    //alertCharArray(s[2]);
    //alertCharArray(s[3]);
    //
    //
    //g_optionArray[0] = "abc";
    //alertCharArray(g_optionArray[0]);
    //alertCharArray(g_optionArray[1]);
    //alertCharArray(g_optionArray[2]);
    //alertCharArray(g_optionArray[3]);
    //alertCharArray(g_optionArray[4]);
    
    // Testing array of char array
    //const char *s[]={"Jan","Feb","Mar","April"};
    //const char **p;
    //size_t i;
    //
    //#define countof(X) ( (size_t) ( sizeof(X)/sizeof*(X) ) )
    //
    //alertCharArray("Loop 1:");
    //for (i = 0; i < countof(s); i++)
    //alertCharArray((char*)(s[i]));
    //
    //alertCharArray("Loop 2:");
    //for (p = s, i = 0; i < countof(s); i++)
    //alertCharArray((char*)(p[i]));
    //
    //alertCharArray("Loop 3:");
    //for (p = s; p < &s[countof(s)]; p++)
    //alertCharArray((char*)(*p));
    




    //setCommand(TRIGGER_SNIPPET_INDEX, TEXT("Trigger Snippet/Navigate to Hotspot"), fingerText, NULL, false);
    //::GenerateKey(VK_TAB, TRUE);

    //ShortcutKey *shKey = new ShortcutKey;
	//shKey->_isAlt = true;
	//shKey->_isCtrl = true;
	//shKey->_isShift = true;
	//shKey->_key = VK_TAB;
    //setCommand(TRIGGER_SNIPPET_INDEX, TEXT("Trigger Snippet/Navigate to Hotspot"), fingerText, shKey, false);
    // create process, no console window
    //STARTUPINFO         si;
    //PROCESS_INFORMATION pi;
    ////TCHAR               cmdLine[MAX_PATH] = L"npp -multiInst";
    //TCHAR               cmdLine[MAX_PATH] = L"dir > d:\\temp.txt";
    //
    //::ZeroMemory(&si, sizeof(si));
    //si.cb = sizeof(si);
    //
    //::CreateProcess(NULL, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
    
    
    //Console::ReadLine();


    //_beginthread( Thread, 0, NULL );

    //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
    //selectionToSnippet();

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

void alertNumber(int input)
{
    wchar_t countText[10];
    ::_itow_s(input, countText, 10, 10);
    ::MessageBox(nppData._nppHandle, countText, TEXT("Trace"), MB_OK);

}
void alertCharArray(char* input)
{
    wchar_t* wcstring;
    convertToWideChar(input, &wcstring);
    
    //size_t origsize = strlen(input) + 1;
    //const size_t newsize = 1000;
    //size_t convertedChars = 0;
    //wchar_t wcstring[newsize];
    //mbstowcs_s(&convertedChars, wcstring, origsize, input, _TRUNCATE);
    ::MessageBox(nppData._nppHandle, wcstring, TEXT("Trace"), MB_OK);
    delete [] wcstring;
}

void alertTCharArray(TCHAR* input)
{
    ::MessageBox(nppData._nppHandle, input, TEXT("Trace"), MB_OK);
}