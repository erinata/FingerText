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

#include "PluginDefinition.h"

FuncItem funcItem[nbFunc];   // The plugin data that Notepad++ needs
NppData nppData;  // The data of Notepad++ that you can use in your plugin commands
SciFnDirect pSciMsg;  // For direct scintilla call
sptr_t pSciWndData;   // For direct scintilla call

sqlite3 *g_db;
bool     g_dbOpen;

struct SnipIndex 
{
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
int g_selectionMonitor;
bool g_rectSelection;

int g_editorLineCount;

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
#define DEFAULT_SNIPPET_LIST_LENGTH 1000
#define DEFAULT_SNIPPET_LIST_ORDER_TAG_TYPE 1
#define DEFAULT_TAB_TAG_COMPLETION 0
#define DEFAULT_LIVE_HINT_UPDATE 1
#define DEFAULT_INDENT_REFERENCE 1
#define DEFAULT_CHAIN_LIMIT 40
#define DEFAULT_PRESERVE_STEPS 0
#define DEFAULT_ESCAPE_CHAR 0
#define DEFAULT_IMPORT_OVERWRITE_OPTION 0
#define DEFAULT_IMPORT_OVERWRITE_CONFIRM 0
#define DEFAULT_INCLUSIVE_TRIGGERTEXT_COMPLETION 0
#define DEFAULT_LIVE_PREVIEW_BOX 1
#define DEFAULT_EDITOR_CARET_BOUND 1
#define DEFAULT_FORCE_MULTI_PASTE 1

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
int g_editorCaretBound;
int g_forceMultiPaste;

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
#define TESTING2_INDEX 18

// Initialize your plugin data here; called while plugin loading   
void pluginInit(HANDLE hModule)
{
    snippetDock.init((HINSTANCE)hModule, NULL);
}

void pluginCleanUp()
{
    //TODO: think about how to save the parameters for the next session during clean up
    g_liveHintUpdate = 0;
    //saveCustomScope();
    //writeConfig();
    
}

// Initialization of plugin commands
void commandMenuInit()
{
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

    setCommand(TRIGGER_SNIPPET_INDEX, TEXT("Trigger Snippet/Navigate to Hotspot"), tabActivate, shKey, false);
    
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
    setCommand(TESTING2_INDEX, TEXT("Testing2"), testing2, NULL, false);
}

bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );

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

// command shortcut clean up
void commandMenuCleanUp()
{
    //delete [] snippetEditTemplate;
    delete funcItem[0]._pShKey;
	// Don't forget to deallocate your shortcut here
}

// Functions for Fingertext
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

void upgradeMessage()
{
    //TODO: make better structure for welcometext. like one piece of string per version.
    //TODO: dynamic upgrade message

    if (g_newUpdate)
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

        char* welcomeText = new char[7000];
        strcpy(welcomeText,"");
        strcat(welcomeText, "Thanks for Upgrading to ");
        strcat(welcomeText, VERSION_TEXT_LONG);
        strcat(welcomeText, "\r\n\
Please read this document if you are upgrading from previous versions.\r\n\r\n\
Upgrading from 0.5.21 or above\r\n\
Everything is compatible.\r\n\r\n\
Upgrading from 0.5.20\r\n\
In version ");
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, " after you triggered a option hotspot, you can use right/down arrow to move to next option, left/up arrow to move to previous option. Hit tab to exit option mode.\r\n\
\r\n\
Upgrading from any version between 0.5.0 and 0.5.18\r\n\
All hotspots are now triggered from inside to outside, left to right. Therefore dynamic snippets that is created before 0.5.20 can behave differently in this version.\r\n\
\r\n\
Upgrading from 0.4.15 or 0.4.16\r\n\
If you want to use your snippets after you upgrade to ");
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, ", you can go to the config folder and move the FingerText.db3 file to the config\\FingerText folder after update.\r\n\
chain snippet is indicated by $[![(cha)]!] instead of $[![(chain)]!]\r\n\
\r\n\
Upgrading from 0.4.4 or 0.4.11\r\n\
If you want to use your snippets after you upgrade to ");
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, ", you can go to the config folder and move the FingerText.db3 file to the config\\FingerText folder after update.\r\n\
\r\n\
Upgrading from 0.4.1\r\n\
If you want to use your snippets after you upgrade to ");
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, ", you can go to the %Notepad++ folder%\\plugins\\FingerText and get the old database file Snippets.db3. Rename is to FingerText.db3 and move it to the config\\FingerText folder after update.\r\n\
\r\n\
Upgrading from 0.3.5 or below\r\n\
FingerText 0.3.5 or below use a 'one snippet per file' system to store snippets, which is not compatibile with current version. If you really have a lot of snippets created using these early version, please send your snippets to erinata@gmail.com. I will try my best to import them into the database in the current version.\r\n\
");

        ::SendMessage(getCurrentScintilla(), SCI_INSERTTEXT, 0, (LPARAM)welcomeText);

        delete [] welcomeText;
    }
}

int searchNext(char* searchText)
{
    ::SendScintilla(SCI_SEARCHANCHOR, 0,0);
    return ::SendScintilla(SCI_SEARCHNEXT, 0,(LPARAM)searchText);
}
int searchPrev(char* searchText)
{
    ::SendScintilla(SCI_SEARCHANCHOR, 0,0); 
    return ::SendScintilla(SCI_SEARCHPREV, 0,(LPARAM)searchText);
}

void selectionToSnippet()
{
    g_selectionMonitor--;
    
    //g_editorCaretBound--;
    
    //HWND curScintilla = getCurrentScintilla();
    int selectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);
    int selectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
    bool withSelection = false;

    char* selection;
    
    if (selectionEnd>selectionStart)
    {
        selection = new char [selectionEnd - selectionStart +1];
        ::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
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
    
    
    //curScintilla = getCurrentScintilla();
    
    //TODO: consider using YES NO CANCEL dialog in promptsavesnippet
    promptSaveSnippet(TEXT("Do you wish to save the current snippet before creating a new one?"));
    
    
    ::SendScintilla(SCI_CLEARALL,0,0);
    ::SendScintilla(SCI_INSERTTEXT,::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)SNIPPET_EDIT_TEMPLATE);
    ::SendScintilla(SCI_INSERTTEXT,::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"triggertext\r\nGLOBAL\r\n");
    ::SendScintilla(SCI_INSERTTEXT,::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)selection);
    ::SendScintilla(SCI_INSERTTEXT,::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"[>END<]");

    g_editorView = 1;
    
    //updateDockItems(false,false);
    //updateMode();
    //refreshAnnotation();
    ::SendScintilla(SCI_GOTOLINE,1,0);
    ::SendScintilla(SCI_WORDRIGHTEXTEND,1,0);

    if (withSelection) delete [] selection;
    ::SendScintilla(SCI_EMPTYUNDOBUFFER,0,0);
    
    //g_editorCaretBound++;
    g_selectionMonitor++;
}

void editSnippet()
{
    int index = snippetDock.getCount() - snippetDock.getSelection()-1;
    char* tempTriggerText;
    char* tempScope;

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
            //HWND curScintilla = getCurrentScintilla();
            promptSaveSnippet(TEXT("Do you wish to save the current snippet before editing anotoher one?"));
            ::SendScintilla(SCI_CLEARALL,0,0);
               
            //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)SNIPPET_EDIT_TEMPLATE);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)tempTriggerText);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"\r\n");
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)tempScope);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"\r\n");

            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)snippetText);

            g_editorView = true;
            refreshAnnotation();
		}
	}
    
	sqlite3_finalize(stmt);

    //HWND curScintilla = getCurrentScintilla();
    ::SendScintilla(SCI_SETSAVEPOINT,0,0);
    ::SendScintilla(SCI_EMPTYUNDOBUFFER,0,0);

    //if (tempTriggerText)
    //{
        //delete [] tempTriggerText; // deleting them cause error
    //}
    //if (tempScope) delete [] tempScope;
}

void deleteSnippet()
{
    //TODO: should scroll back to original position after delete
    //HWND curScintilla = getCurrentScintilla();
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

bool getLineChecked(char **buffer, int lineNumber, TCHAR* errorText)
{
    // TODO: and check for more error, say the triggertext has to be one word
    bool problemSnippet = false;

    ::SendScintilla(SCI_GOTOLINE,lineNumber,0);

    int tagPosStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);

    int tagPosEnd;
    
    if (lineNumber == 3)
    {
        tagPosEnd = ::SendScintilla(SCI_GETLENGTH,0,0);
    } else
    {
        int tagPosLineEnd = ::SendScintilla(SCI_GETLINEENDPOSITION,lineNumber,0);

        char* wordChar;
        if (lineNumber==2)
        {
            wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.:";
            
        } else //if (lineNumber==1)
        {
            wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.";
        }
        ::SendScintilla(SCI_SETWORDCHARS, 0, (LPARAM)wordChar);
        tagPosEnd = ::SendScintilla(SCI_WORDENDPOSITION,tagPosStart,0);
        ::SendScintilla(SCI_SETCHARSDEFAULT, 0, 0);
        //::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        //::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" ");
        //tagPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        if ((tagPosEnd>tagPosLineEnd) || (tagPosEnd-tagPosStart<=0))
        {
            //blank
            ::SendScintilla(SCI_GOTOLINE,lineNumber,0);
            ::MessageBox(nppData._nppHandle, errorText, TEXT("FingerText"), MB_OK);
            problemSnippet = true;
            
        } else if (tagPosEnd<tagPosLineEnd)
        {
            // multi
            ::SendScintilla(SCI_GOTOLINE,lineNumber,0);
            ::MessageBox(nppData._nppHandle, errorText, TEXT("FingerText"), MB_OK);
            problemSnippet = true;
        }
    }

    if (lineNumber == 3)
    {
        ::SendScintilla(SCI_GOTOPOS,tagPosStart,0);
        int spot = searchNext("[>END<]");
        if (spot<0)
        {
            ::MessageBox(nppData._nppHandle, TEXT("You should put an \"[>END<]\" (without quotes) at the end of your snippet content."), TEXT("FingerText"), MB_OK);
            problemSnippet = true;
        }
    }

    ::SendScintilla(SCI_SETSELECTION,tagPosStart,tagPosEnd);
    *buffer = new char[tagPosEnd-tagPosStart + 1];
    ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*buffer));

    return problemSnippet;
}

//TODO: saveSnippet() and importSnippet() need refactoring sooooooo badly..................
void saveSnippet()
{
    //HWND curScintilla = getCurrentScintilla();
    g_selectionMonitor--;
    int docLength = ::SendScintilla(SCI_GETLENGTH,0,0);
    // insert a space at the end of the doc so the ::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" "); will not get into error
    // TODO: Make sure that it is not necessary to keep this line
    //::SendMessage(curScintilla, SCI_INSERTTEXT, docLength, (LPARAM)" ");
    
    bool problemSnippet = false;

    char* tagText;
    char* tagTypeText;
    char* snippetText;

    if (getLineChecked(&tagText,1,TEXT("TriggerText cannot be blank, and it can only contain alphanumeric characters (no spaces allowed)"))==true) problemSnippet = true;
    if (getLineChecked(&tagTypeText,2,TEXT("Scope cannot be blank, and it can only contain alphanumeric characters and/or period."))==true) problemSnippet = true;
    if (getLineChecked(&snippetText,3,TEXT("Snippet Content cannot be blank."))==true) problemSnippet = true;
    
    ::SendScintilla(SCI_SETSELECTION,docLength,docLength+1); //Take away the extra space added
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");

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
                    ::SendScintilla(SCI_SETSELECTION, 0, 1);
                    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"-");
                    ::SendScintilla(SCI_GOTOPOS, 0, 0);
                
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
        ::SendScintilla(SCI_SETSAVEPOINT,0,0);
    }
    delete [] tagText;
    delete [] tagTypeText;
    delete [] snippetText;
    
    updateDockItems(false,false);
    g_selectionMonitor++;
}


sptr_t SendScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return pSciMsg(pSciWndData, iMessage, wParam, lParam);
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

void restoreTab(int &posCurrent, int &posSelectionStart, int &posSelectionEnd)
{
    // restoring the original tab action
    ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
    ::SendScintilla(SCI_SETSELECTION,posSelectionStart,posSelectionEnd);
    ::SendScintilla(SCI_TAB,0,0);	
}

// TODO: refactor the dynamic hotspot functions
bool dynamicHotspot(int &startingPos)
{
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
        ::SendScintilla(SCI_GOTOPOS,checkPoint,0);
        spot = searchNext(tagTail);   // Find the tail first so that nested snippets are triggered correctly
        //spot = searchNext(curScintilla, tagSign);
        if (spot>=0)
	    {
            checkPoint = ::SendScintilla(SCI_GETCURRENTPOS,0,0)+1;
            spotComplete = -1;
            spotComplete = searchPrev(tagSign);
            if (spotComplete>=0)
            {

                int firstPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                int secondPos = 0;
                spotType = grabHotSpotContent(&hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength,true);
                
                ///////////////////
                //TODO: checkPoint = firstPos; not needed?
                if (spotType == 1)
                {
                    checkPoint = firstPos;
                    chainSnippet(firstPos, hotSpotText+5);
                    
                    limitCounter++;
                } else if (spotType == 2)
                {
                    checkPoint = firstPos;
                    keyWordSpot(firstPos,hotSpotText+5, startingPos, checkPoint);
                    
                    limitCounter++;
                } else if (spotType == 3)
                {
                    checkPoint = firstPos;
                    executeCommand(firstPos, hotSpotText+5);
                    
                    limitCounter++;
                } else if (spotType == 4)
                {
                    checkPoint = firstPos;
                    launchMessageBox(firstPos,hotSpotText+5);

                    limitCounter++;
                }
                else
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

void chainSnippet(int &firstPos, char* hotSpotText)
{
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_GOTOPOS,triggerPos,0);
    triggerTag(triggerPos,false, strlen(hotSpotText));
}


//void executeCommand(int &firstPos, char* hotSpotText)
//{
//    int triggerPos = strlen(hotSpotText)+firstPos;
//    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
//    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
//    
//    char  psBuffer[130];
//    FILE   *pPipe;
//    int resultLength;
//    //TODO: try the createprocess instead of _popen?
//    //http://msdn.microsoft.com/en-us/library/ms682499(v=vs.85).aspx
//
//    //pPipe = _popen( "ruby -e 'puts 1+1'", "rt" );
//    if( (pPipe = _popen( hotSpotText, "rt" )) == NULL )
//    {    
//        return;
//    }
//
//    ::memset(psBuffer,0,sizeof(psBuffer));
//
//    while(fgets(psBuffer, 129, pPipe))
//    {
//        ::SendScintilla(SCI_REPLACESEL, 128, (LPARAM)psBuffer);
//        ::memset (psBuffer,0,sizeof(psBuffer));
//    }
//    _pclose( pPipe );
//}

void executeCommand(int &firstPos, char* hotSpotText)
{

    //HWND curScintilla = getCurrentScintilla();

    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
    //::SendMessage(curScintilla, SCI_SETSEL,firstPos,triggerPos);
    //::SendMessage(curScintilla, SCI_REPLACESEL,0,(LPARAM)"");
    
    HANDLE processStdinRead = NULL;
    HANDLE processStdinWrite = NULL;

    HANDLE processStdoutRead = NULL;
    HANDLE processStdoutWrite = NULL;

    SECURITY_ATTRIBUTES securityAttributes;

    securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    securityAttributes.bInheritHandle = true;
    securityAttributes.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&processStdoutRead, &processStdoutWrite, &securityAttributes, 0) )
    {
        //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->StdoutRd CreatePipe\n");
    }

    if (!SetHandleInformation(processStdoutRead, HANDLE_FLAG_INHERIT, 0) )
    {
        //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->Stdout SetHandleInformation\n");
    }

    if (!CreatePipe(&processStdinRead, &processStdinWrite, &securityAttributes, 0))
    {
        //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->Stdin CreatePipe\n");
    }

   if (!SetHandleInformation(processStdinWrite, HANDLE_FLAG_INHERIT, 0) )
   {
       //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->Stdin SetHandleInformation\n");
   }

    //TCHAR CMD_LINE[bufSize];
    //
    //int len = MultiByteToWideChar ((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, hotSpotText, -1, NULL, 0);
    //MultiByteToWideChar ((int)::SendMessage(curScintilla, SCI_GETCODEPAGE, 0, 0), 0, hotSpotText, -1, CMD_LINE, len);
    
    TCHAR* cmdLine;
    convertToWideChar(hotSpotText,&cmdLine);

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    bool processSuccess = false;

    ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &si, sizeof(STARTUPINFO) );

    si.cb = sizeof(STARTUPINFO);
    si.hStdError = processStdoutWrite;
    si.hStdOutput = processStdoutWrite;
    si.hStdInput = processStdinRead;
    // Hide window
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;

    // Create process.
    processSuccess = CreateProcess(
        NULL,
        cmdLine,  // command line
        NULL,     // process security attributes
        NULL,    // primary thread security attributes
        true,     // handles are inherited
        0,        // creation flags
        NULL,     // use parent's environment
        NULL,     // use parent's current directory
        &si,      // STARTUPINFO pointer
        &pi       // receives PROCESS_INFORMATION
        );     

    if (!processSuccess)
    {
        //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->Error in CreateProcess\n");
        char* hotSpotTextCmd = new char[strlen(hotSpotText)+8];
        strcpy(hotSpotTextCmd, "cmd /c ");
        strcat(hotSpotTextCmd,hotSpotText);
        //strcpy(hotSpotTextCmd, hotSpotText);
        TCHAR* cmdLine2;
        convertToWideChar(hotSpotTextCmd,&cmdLine2);

        processSuccess = CreateProcess(
        NULL,
        cmdLine2,  // command line
        NULL,     // process security attributes
        NULL,    // primary thread security attributes
        true,     // handles are inherited
        0,        // creation flags
        NULL,     // use parent's environment
        NULL,     // use parent's current directory
        &si,      // STARTUPINFO pointer
        &pi       // receives PROCESS_INFORMATION
        );

        if (!processSuccess)
        {
            //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->Error in CreateProcess\n");
        }
        else
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        delete [] cmdLine2;
        delete [] hotSpotTextCmd;
    

    }
    else
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    delete [] cmdLine;
    
    //TODO: option to skip the output part (so that we can run a program that didn't return immediately but still not freezing up npp)
    const int bufSize = 100;
    DWORD read;
    char buffer[bufSize];
    bool readSuccess = false;

    if (!CloseHandle(processStdoutWrite))
    {
        //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)"->StdOutWr CloseHandle\n");
    }

    //::Sleep(100);  //this is temporary solution to the incorrectly written output to npp.....

    //HWND curScintilla = getCurrentScintilla();
    while (1)
    {   
        readSuccess = ReadFile(processStdoutRead, buffer, bufSize - 1, &read, NULL);
        
        if (!readSuccess || read == 0 ) break;

        buffer[read] = '\0';
        
        //::SendMessage(curScintilla, SCI_REPLACESEL, bufSize - 1, (LPARAM)Buffer);
        ::SendScintilla(SCI_REPLACESEL, bufSize - 1, (LPARAM)buffer);
        //::SendScintilla(SCI_INSERTTEXT, firstPos, (LPARAM)Buffer);

        if (!readSuccess ) break;
    }

   //::SendScintilla(SCI_INSERTTEXT, 0, (LPARAM)"->End of process execution.\n");
}

void launchMessageBox(int &firstPos, char* hotSpotText)
{
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);

    char* getTerm;
    getTerm = new char[strlen(hotSpotText)];
    strcpy(getTerm,"");
    
    // TODO: probably can just translate the text like "MB_OK" to the corresponding number and send it to the messagebox message directly. In this case people can just follow microsofe documentation.   
    int messageType = MB_OK;
    if (strncmp(hotSpotText,"OK:",3)==0) 
    {
        messageType = MB_OK;
        strcpy(getTerm,hotSpotText+3);
    } else if (strncmp(hotSpotText,"YESNO:",6)==0) 
    {
        messageType = MB_YESNO;
        strcpy(getTerm,hotSpotText+6);
    } else if (strncmp(hotSpotText,"OKCANCEL:",9)==0) 
    {
        messageType = MB_OKCANCEL;
        strcpy(getTerm,hotSpotText+9);
    } else if (strncmp(hotSpotText,"ABORTRETRYIGNORE:",17)==0) 
    {
        messageType = MB_ABORTRETRYIGNORE;
        strcpy(getTerm,hotSpotText+17);
    } else if (strncmp(hotSpotText,"CANCELTRYCONTINUE:",18)==0) 
    {
        messageType = MB_CANCELTRYCONTINUE;
        strcpy(getTerm,hotSpotText+18);
    } else if (strncmp(hotSpotText,"RETRYCANCEL:",12)==0) 
    {
        messageType = MB_RETRYCANCEL;
        strcpy(getTerm,hotSpotText+12);
    } else if (strncmp(hotSpotText,"YESNOCANCEL:",12)==0) 
    {
        messageType = MB_YESNOCANCEL;
        strcpy(getTerm,hotSpotText+12);
    } else
    {
        delete [] getTerm;
    //TODO: Confirm to use return here, it stops the msg box launching when the msgbox type is unknown
        return;
    }

    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
    TCHAR* getTermWide;
    convertToWideChar(getTerm,&getTermWide);
    int retVal = 0;
    retVal = ::MessageBox(nppData._nppHandle, getTermWide, TEXT("FingerText"), messageType);
    char countText[10];
    ::_itoa(retVal, countText, 10);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)countText);
    delete [] getTerm;
    delete [] getTermWide;
}

void keyWordSpot(int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint)
{
    
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
    //TODO: At least I should rearrange the keyword a little bit for efficiency
    if (strcmp(hotSpotText,"PASTE")==0)
    {
        ::SendScintilla(SCI_PASTE,0,0);
	    
    } else if (strcmp(hotSpotText,"DATE")==0)
    {
        char* dateText = getDateTime(NULL,true,DATE_LONGDATE);
        ::SendScintilla( SCI_REPLACESEL, 0, (LPARAM)dateText);
        delete [] dateText;
        //insertDateTime(true,DATE_LONGDATE,curScintilla);
        
    } else if (strcmp(hotSpotText,"TIME")==0)
    {
        char* timeText = getDateTime(NULL,false,0);
        ::SendScintilla( SCI_REPLACESEL, 0, (LPARAM)timeText);
        delete [] timeText;
        //insertDateTime(false,0,curScintilla);
    } else if (strcmp(hotSpotText,"FILENAME")==0)
    {
        insertNppPath(NPPM_GETNAMEPART);
        
    } else if (strcmp(hotSpotText,"EXTNAME")==0)
    {
        insertNppPath(NPPM_GETEXTPART);
        
    } else if (strcmp(hotSpotText,"DIRECTORY")==0)
    {
        insertNppPath(NPPM_GETCURRENTDIRECTORY);
    //} else if (strcmp(hotSpotText,"GETSCRIPT")==0) 
    } else if (strcmp(hotSpotText,"TEMPFILE")==0)
    {
        insertPath(g_fttempPath);

    } else if (strncmp(hotSpotText,"GET:",4)==0) 
    {
        //TODO: lots of issues in the GET and CUT keywords, when nothing can be cut
        //TODO: write a function to get the command and parameter sepearately. or turn this whole thing into a new type of hotspot
        //TODO: a complete rewrite of GET, GET:, GETALL, GETLINE, CUT, CUT:
        
        emptyFile(g_fttempPath);
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+4);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        int scriptFound = -1;
        if (strlen(getTerm)>0)
        {
            ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
            scriptFound = searchPrev(getTerm);
        }
        delete [] getTerm;

        int scriptStart = 0;
        if (scriptFound>=0)
        {
            scriptStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        }
            
        int selectionEnd = startingPos-1; // -1 because the space before the snippet tag should not be included
        int selectionStart = scriptStart+strlen(hotSpotText)-4;
        if (selectionEnd>=selectionStart)
        {
            ::SendScintilla(SCI_SETSEL,selectionStart,selectionEnd);
            char* selection = new char [selectionEnd - selectionStart +1];
            ::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
            ::SendScintilla(SCI_SETSEL,scriptStart,selectionEnd+1); //+1 to make up the -1 in setting the selection End
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
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
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_SETSEL,startingPos-1,startingPos);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
        ::SendScintilla(SCI_WORDLEFTEXTEND,0,0);
        char* selection = new char [startingPos -1 - ::SendScintilla(SCI_GETCURRENTPOS,0,0) +1];
        ::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
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
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_SETSEL,startingPos-1,startingPos);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
        ::SendScintilla(SCI_HOMEEXTEND,0,0);
        char* selection = new char [startingPos -1 - ::SendScintilla(SCI_GETCURRENTPOS,0,0) +1];
        ::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
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
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        int scriptFound = -1;
        if (strlen(getTerm)>0)
        {
            ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
            scriptFound = searchPrev(getTerm);
        }
        delete [] getTerm;
        // TODO: still a bug when triggered from the beginning of the document with a searching term. For example: $[![(key)CUT: ]!]$[![(key)SCOPE:$[![(key)PASTE]!]]!][>END<] (testscope) will get 1 missing character when triggering from start of the document
        // TODO: same problem for GET:
        int scriptStart = 0;
        if (scriptFound>=0)
        {
            scriptStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        }
        int selectionEnd = startingPos-1; // -1 because the space before the snippet tag should not be included
        int selectionStart = scriptStart+strlen(hotSpotText)-4;
        if (selectionEnd>=selectionStart)
        {
            ::SendScintilla(SCI_SETSEL,selectionStart,selectionEnd);
            ::SendScintilla(SCI_COPY,0,0);
            ::SendScintilla(SCI_SETSEL,scriptStart,selectionEnd+1); //+1 to make up the -1 in setting the selection End
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
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
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_SETSEL,startingPos-1,startingPos);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
        ::SendScintilla(SCI_WORDLEFTEXTEND,0,0);
        startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        ::SendScintilla(SCI_CUT,0,0);
    } else if (strcmp(hotSpotText,"CUTLINE")==0)
    {
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_SETSEL,startingPos-1,startingPos);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_GOTOPOS,startingPos-1,0);
        ::SendScintilla(SCI_HOMEEXTEND,0,0);
        startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        checkPoint = startingPos;
        ::SendScintilla(SCI_CUT,0,0);
    } else if (strncmp(hotSpotText,"UPPER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+6);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)::_strupr(getTerm));
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"LOWER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+6);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)::_strlwr(getTerm));
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"TIME:",5)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+5);
        char* timeReturn = getDateTime(getTerm,false);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)timeReturn);
        delete [] timeReturn;
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"DATE:",5)==0)
    {
        char* getTerm;
        getTerm = new char[strlen(hotSpotText)];
        strcpy(getTerm,hotSpotText+5);
        char* dateReturn = getDateTime(getTerm);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)dateReturn);
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
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
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
    //    ::SendScintilla( SCI_REPLACESEL, 0, (LPARAM)dateText);
    //    delete [] dateText;
    //    //insertDateTime(true,DATE_SHORTDATE,curScintilla);
	//    
    //} 
    //else if (strcmp(hotSpotText,"TIMESHORT")==0)
    //{
    //    char* timeText = getDateTime(NULL,false,TIME_NOSECONDS);
    //    ::SendScintilla( SCI_REPLACESEL, 0, (LPARAM)timeText);
    //    delete [] timeText;
    //    //insertDateTime(false,TIME_NOSECONDS,curScintilla);
    //    
    //} 
}

//TODO: insertpath and insertnpppath (and/or other insert function) need refactoring
void insertPath(TCHAR* path)
{
    char pathText[MAX_PATH];
	WideCharToMultiByte((int)::SendScintilla(SCI_GETCODEPAGE, 0, 0), 0, path, -1, pathText, MAX_PATH, NULL, NULL);
    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)pathText);
}

void insertNppPath(int msg)
{
    //TODO: use converttowidechar() ?
	TCHAR path[MAX_PATH];
	::SendMessage(nppData._nppHandle, msg, 0, (LPARAM)path);

	char pathText[MAX_PATH];
	WideCharToMultiByte((int)::SendScintilla(SCI_GETCODEPAGE, 0, 0), 0, path, -1, pathText, MAX_PATH, NULL, NULL);
	::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)pathText);
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
    //HWND curScintilla = getCurrentScintilla();

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

bool hotSpotNavigation()
{
    // TODO: consolidate this part with dynamic hotspots? 

    char tagSign[] = "$[![";
    //int tagSignLength = strlen(tagSign);
    int tagSignLength = 4;
    char tagTail[] = "]!]";
    //int tagTailLength = strlen(tagTail);

    char *hotSpotText;
    char *hotSpot;

    int tagSpot = searchNext(tagTail);    // Find the tail first so that nested snippets are triggered correctly
	if (tagSpot>=0)
	{
        if (g_preserveSteps==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);

        if (searchPrev(tagSign)>=0)
        {
            int firstPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            int secondPos = 0;
            grabHotSpotContent(&hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength,false);
            
            ::SendScintilla(SCI_GOTOPOS,firstPos,0);

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
                    ::SendScintilla(SCI_GOTOPOS,tempOptionStart,0);
                    optionFound = searchNext("|~|");
                    if ((optionFound>=0) && (::SendScintilla(SCI_GETCURRENTPOS,0,0)<secondPos))
                    {
                        tempOptionEnd = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                        ::SendScintilla(SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                        char* optionText;
                        optionText = new char[tempOptionEnd - tempOptionStart + 1];
                        ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                        addOptionItem(optionText);
                        i++;
                    } else
                    {
                        tempOptionEnd = secondPos-4;
                        ::SendScintilla(SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                        char* optionText;
                        optionText = new char[tempOptionEnd - tempOptionStart + 1];
                        ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                        addOptionItem(optionText);
                        i++;
                        
                        break;
                    }
                };

                //g_optionOperating = true; 

                ::SendScintilla(SCI_SETSELECTION,firstPos,tempOptionEnd);
                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);
                //g_optionOperating = false; 

                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                g_optionStartPosition = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                g_optionEndPosition = g_optionStartPosition + strlen(g_optionArray[g_optionCurrent]);
                ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
                if (i>1)
                {
                    g_optionMode = true;
                    updateOptionCurrent(true);
                }
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
                
                    hotSpotFound = searchNext(hotSpot);
                    if ((hotSpotFound>=0) && strlen(hotSpotText)>0)
                    {
                        tempPos[i] = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
                        ::SendScintilla(SCI_GOTOPOS,tempPos[i],0);
                    } else
                    {
                        break;
                        //tempPos[i]=-1;
                    }
                }
                //::SendMessage(curScintilla,SCI_GOTOPOS,::SendMessage(curScintilla,SCI_POSITIONFROMLINE,posLine,0),0);
                //::SendMessage(curScintilla,SCI_GOTOLINE,posLine,0);

                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                ::SendScintilla(SCI_SCROLLCARET,0,0);
                
                ::SendScintilla(SCI_SETSELECTION,firstPos,secondPos-tagSignLength);
                for (int j=1;j<i;j++)
                {
                    if (tempPos[j]!=-1)
                    {
                        ::SendScintilla(SCI_ADDSELECTION,tempPos[j],tempPos[j]+(secondPos-tagSignLength-firstPos));
                    }
                }
                ::SendScintilla(SCI_SETMAINSELECTION,0,0);
                ::SendScintilla(SCI_LINESCROLL,0,0);
            }

            delete [] hotSpot;
            delete [] hotSpotText;

            if (g_preserveSteps==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
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

int grabHotSpotContent(char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength, bool dynamic)
{
    //TODO: examine whether the bool dynamic is still needed, or we just use the (cha) (key) (cmd) and (opt) to determine what should happen
    
    int spotType = 0;

    searchNext("]!]");
	secondPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);

    ::SendScintilla(SCI_SETSELECTION,firstPos+signLength,secondPos);

    *hotSpotText = new char[secondPos - (firstPos + signLength) + 1];
    ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpotText));

    if (strncmp(*hotSpotText,"(cha)",5)==0)
    {
        spotType = 1;
        //alertCharArray("key");
    } else if (strncmp(*hotSpotText,"(key)",5)==0)
    {
        spotType = 2;
        //alertCharArray("cha");
    } else if ((strncmp(*hotSpotText,"(run)",5)==0) || (strncmp(*hotSpotText,"(cmd)",5)==0))  //TODO: the command hotspot is renamed to (run) this line is for keeping backward compatibility
    {
        spotType = 3;
        //alertCharArray("cmd");
    } else if (strncmp(*hotSpotText,"(msg)",5)==0) //TODO: should think more about this hotspot, It can be made into a more general (ask) hotspot....so keep this feature private for the moment
    {
        spotType = 4;
    }

    ::SendScintilla(SCI_SETSELECTION,firstPos,secondPos+3);
    
    *hotSpot = new char[secondPos+3 - firstPos + 1];
    ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpot));

    // TODO: consider moving this part to the function calling grabHotSpotContent, this facilitates at least the implementation of (nor) hotspot
    if ((spotType>0) && (dynamic))
    {
        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)*hotSpotText+5);
    } else if (!dynamic)
    {
        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)*hotSpotText);
    }
    ::SendScintilla(SCI_GOTOPOS,secondPos+3,0);
    
    return spotType;
    //return secondPos;  
}

void showPreview(bool top)
{
    //TODO: Testing using LB_SETANCHORINDEX LB_GETANCHORINDEX LB_SETCARETINDEX LB_GETCARETINDEX to change the selection in list box 
    sqlite3_stmt *stmt;
    int index = 0;

    //HWND curScintilla = getCurrentScintilla();
    int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);

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
                
                char* previewText = new char[500];
                strcpy(previewText,"[");
                strcat(previewText, g_snippetCache[index].triggerText);//TODO: showing the triggertext on the title "snippet preview" instead
                strcat(previewText,"]:\r\n");
                char* contentTruncated = new char[155];
                strncpy(contentTruncated, snippetText, 154);
                contentTruncated[154]='\0';
                //strcat(contentTruncated,"\0");
                strcat(previewText,contentTruncated);
                if (strlen(contentTruncated)>=153) strcat(previewText, ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . "); 
                
                TCHAR* previewTextWide;
                convertToWideChar(previewText,&previewTextWide);
                //TODO: investigate why all eol are messed up in preview box

                //size_t origsize = strlen(snippetText) + 1; 
                //size_t convertedChars = 0; 
                //wchar_t previewText[270]; 
                //mbstowcs_s(&convertedChars, previewText, 190, snippetText, _TRUNCATE);
                //
                //if (convertedChars>=184)
                //{
                //    const TCHAR etcText[] = TEXT(" . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .");
                //    ::_tcscat_s(previewText,etcText);
                //}
                
                snippetDock.setDlgText(ID_SNIPSHOW_EDIT,previewTextWide);

                delete [] previewText;
                delete [] contentTruncated;
                delete [] previewTextWide;

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
    //HWND curScintilla = getCurrentScintilla();
    int posStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tagSign);
    //int posCurrent = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
    ::SendScintilla(SCI_GOTOPOS,posStart+4,0);
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


bool replaceTag(char *expanded, int &posCurrent, int &posBeforeTag)
{
    
    //TODO: can use ::SendMessage(curScintilla, SCI_ENSUREVISIBLE, line-1, 0); to make sure that caret is visible after long snippet substitution.

    //::MessageBox(nppData._nppHandle, TEXT("replace tag"), TEXT("Trace"), MB_OK); 
    //std::streamoff sniplength;
    int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION, posCurrent, 0);
    int initialIndent = ::SendScintilla(SCI_GETLINEINDENTATION, lineCurrent, 0);

    ::SendScintilla(SCI_INSERTTEXT, posCurrent, (LPARAM)"____`[SnippetInserting]");

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
  
	::SendScintilla(SCI_SETTARGETSTART, posBeforeTag, 0);
	::SendScintilla(SCI_SETTARGETEND, posCurrent, 0);
    ::SendScintilla(SCI_REPLACETARGET, strlen(expanded), reinterpret_cast<LPARAM>(expanded));

    searchNext("`[SnippetInserting]");
    int posEndOfInsertedText = ::SendScintilla(SCI_GETCURRENTPOS,0,0)+19;

    // adjust indentation according to initial indentation
    if (g_indentReference==1)
    {
        int lineInsertedSnippet = ::SendScintilla(SCI_LINEFROMPOSITION, posEndOfInsertedText, 0);

        int lineIndent=0;
        for (int i=lineCurrent+1;i<=lineInsertedSnippet;i++)
        {
            lineIndent = ::SendScintilla(SCI_GETLINEINDENTATION, i, 0);
            ::SendScintilla(SCI_SETLINEINDENTATION, i, initialIndent+lineIndent);
        }
    }
    searchNext("`[SnippetInserting]");
    posEndOfInsertedText = ::SendScintilla(SCI_GETCURRENTPOS,0,0)+19;
                    
    ::SendScintilla(SCI_GOTOPOS,posBeforeTag,0);
    searchNext("[>END<]");
    int posEndOfSnippet = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        
    ::SendScintilla(SCI_SETSELECTION,posEndOfSnippet,posEndOfInsertedText);
    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"");
	::SendScintilla(SCI_GOTOPOS, posCurrent, 0);
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

void initialize()
{
    
    g_modifyResponse = true;
    g_enable = true;
    g_customScope = new TCHAR[MAX_PATH];
    g_customEscapeChar = new TCHAR[MAX_PATH];
    g_selectionMonitor = 1;
    g_rectSelection = false;

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
    ::_tcscat_s(path,TEXT("\\FingerText"));
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

    ::_tcscpy_s(g_iniPath,path);
    ::_tcscpy_s(g_ftbPath,path);
    ::_tcscpy_s(g_fttempPath,path);
    //::_tcscpy(g_groupPath,path);
    ::_tcscat_s(g_iniPath,TEXT("\\FingerText.ini"));
    ::_tcscat_s(g_ftbPath,TEXT("\\SnippetEditor.ftb"));
    ::_tcscat_s(g_fttempPath,TEXT("\\FingerText.fttemp"));
    //::_tcscat(g_groupPath,TEXT("\\SnippetGroup.ini"));
    
    setupConfigFile();

    g_snippetCache = new SnipIndex [g_snippetListLength];
    if (PathFileExists(g_ftbPath) == FALSE) emptyFile(g_ftbPath);
    if (PathFileExists(g_fttempPath) == FALSE) emptyFile(g_fttempPath);

    //TODO: better arrangement for this multipaste setting
    if (g_forceMultiPaste) ::SendScintilla(SCI_SETMULTIPASTE,1,0); 
    //updateDockItems(false,false);
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
    g_editorCaretBound = DEFAULT_EDITOR_CARET_BOUND;
    g_forceMultiPaste = DEFAULT_FORCE_MULTI_PASTE;

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
    writeConfigText(g_editorCaretBound,TEXT("editor_caret_bound"));
    writeConfigText(g_forceMultiPaste,TEXT("force_multipaste"));
    
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
    g_editorCaretBound = GetPrivateProfileInt(TEXT("FingerText"), TEXT("editor_caret_bound"), DEFAULT_EDITOR_CARET_BOUND, g_iniPath);
    g_forceMultiPaste = GetPrivateProfileInt(TEXT("FingerText"), TEXT("force_multipaste"), DEFAULT_FORCE_MULTI_PASTE, g_iniPath);

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
        //writeConfig();// TODO: Confirm that this line is not needed
        g_newUpdate = false;
        
    } else if ((g_version >= VERSION_KEEP_CONFIG_START) && (g_version <= VERSION_KEEP_CONFIG_END))// for version changes that do not want to reset database
    {
        g_version = VERSION_LINEAR;
        writeConfigText(g_version,TEXT("version"));
                
        loadConfig(); 
        writeConfig(); 
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


int getCurrentTag(int posCurrent, char **buffer, int triggerLength)
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

        ::SendScintilla(SCI_SETWORDCHARS, 0, (LPARAM)wordChar);
	    posBeforeTag = static_cast<int>(::SendScintilla(SCI_WORDSTARTPOSITION, posCurrent, 1));
        ::SendScintilla(SCI_SETCHARSDEFAULT, 0, 0);
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

	    ::SendScintilla(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tagRange));
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
    if ((!g_editorView) && (g_liveHintUpdate==1) && (g_rectSelection==false))
    {
        if (snippetDock.isVisible())
        {
            g_liveHintUpdate=0;
            //HWND curScintilla = getCurrentScintilla();
            if ((::SendScintilla(SCI_GETMODIFY,0,0)!=0) && (::SendScintilla(SCI_SELECTIONISRECTANGLE,0,0)==0))
            {
                int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                char *partialTag;
	            int tagLength = getCurrentTag(posCurrent, &partialTag);
                
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
    //if (g_modifyResponse) refreshAnnotation();
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
    //TODO: there is a bug in the withAll option. The list is limited by the g_snippetlistlength, which is not a desirable effect

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
        char *customScope = new char[MAX_PATH];
        
        char *tagType1 = NULL;
        TCHAR *fileType1 = new TCHAR[MAX_PATH];
        char *tagType2 = NULL;
        TCHAR *fileType2 = new TCHAR[MAX_PATH];

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
                const char* tempTrigger = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                if ((withAll) || (tempTrigger[0]!='_'))
                {
                    tempTriggerLength = strlen(tempTrigger)*4 + 1;
                    if (tempTriggerLength> triggerLength)
                    {
                        triggerLength = tempTriggerLength;
                    }
                    g_snippetCache[row].triggerText = new char[strlen(tempTrigger)*4 + 1];
                    strcpy(g_snippetCache[row].triggerText, tempTrigger);

                    const char* tempScope = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                    tempScopeLength = strlen(tempScope)*4 + 1;
                    if (tempScopeLength> scopeLength)
                    {
                        scopeLength = tempScopeLength;
                    }
                    g_snippetCache[row].scope = new char[strlen(tempScope)*4 + 1];
                    strcpy(g_snippetCache[row].scope, tempScope);



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
    //::SendMessage(getCurrentScintilla(),SCI_GRABFOCUS,0,0);   
    
}

void populateDockItems()
{
    //TODO: Use 2 columns of list box, or list control
    
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

        ::SendScintilla(SCI_SETCURSOR, SC_CURSORWAIT, 0);
        g_snippetListLength = 100000;
        g_snippetCache = new SnipIndex [g_snippetListLength];
        updateDockItems(true,true,"%");
        
        int exportCount = 0;
        for (int j=0;j<g_snippetListLength;j++)
        {
            if (g_snippetCache[j].scope !=NULL)
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].triggerText);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\r\n");
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].scope);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\r\n");
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)g_snippetCache[j].content);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"!$[FingerTextData FingerTextData]@#\r\n");
                exportCount++;
            }
        }
        ::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILEAS, 0, (LPARAM)fileName);
        success = true;

        ::SendScintilla(SCI_SETCURSOR, SC_CURSORNORMAL, 0);
        wchar_t exportCountText[35] = TEXT("");

        if (exportCount>1)
        {
            ::_itow_s(exportCount, exportCountText, 10, 10);
            wcscat_s(exportCountText,TEXT(" snippets are exported."));
        } else if (exportCount==1)
        {
            wcscat_s(exportCountText,TEXT("1 snippet is exported."));
        } else
        {
            wcscat_s(exportCountText,TEXT("No snippets are exported."));
        }
        
        ::SendScintilla(SCI_SETSAVEPOINT,0,0);
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

//void cleanupString2(char* str, char key)
//{
//    char *from, *to;
//    from=to=str;
//
//    while ((*from != key) && (*to++=*from),*from++);
//    //return str;
//}


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
        
            //HWND curScintilla = getCurrentScintilla();
            ::SendScintilla(SCI_SETCURSOR, SC_CURSORWAIT, 0);

            //::SendMessage(curScintilla, SCI_SETCODEPAGE,65001,0);
            ::SendScintilla(SCI_SETTEXT, 0, (LPARAM)fileText);
            ::SendScintilla(SCI_GOTOPOS, 0, 0);
            ::SendScintilla(SCI_NEWLINE, 0, 0);

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
                ::SendScintilla(SCI_GOTOPOS, 0, 0);
                                
                getLineChecked(&tagText,1,TEXT("Error: Invalid TriggerText. The ftd file may be corrupted."));
                getLineChecked(&tagTypeText,2,TEXT("Error: Invalid Scope. The ftd file may be corrupted."));
                
                // Getting text after the 3rd line until the tag !$[FingerTextData FingerTextData]@#
                ::SendScintilla(SCI_GOTOLINE,3,0);
                snippetPosStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                //int snippetPosEnd = ::SendMessage(curScintilla,SCI_GETLENGTH,0,0);
            
                searchNext("!$[FingerTextData FingerTextData]@#");
                snippetPosEnd = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                ::SendScintilla(SCI_SETSELECTION,snippetPosStart,snippetPosEnd);
            
                snippetText = new char[snippetPosEnd-snippetPosStart + 1];
                ::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(snippetText));
            
                ::SendScintilla(SCI_SETSELECTION,0,snippetPosEnd+1); // This +1 corrupt the ! in !$[FingerTextData FingerTextData]@# so that the program know a snippet is finished importing
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");

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
                            //delete [] snippetTextOld;
                            notOverWrite = true;
                            //sqlite3_finalize(stmt);
                        } else
                        {
                            //delete [] snippetTextOld;
                        //    sqlite3_finalize(stmt);
                            if (conflictKeepCopy==IDNO)
                            {
                                if (g_importOverWriteConfirm == 1)
                                {
                                    // TODO: may be moving the message to earlier location so that the text editor will be showing the message that is about to be overwriting into the database
                                    // TODO: try showing the conflict message on the editor
                        
                                    ::SendScintilla(SCI_GOTOLINE,0,0);
                                    //TODO: refactor this repeated replacesel action
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\r\nConflicting Snippet: \r\n\r\n     ");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tagText);
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"  <");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tagTypeText);
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)">\r\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n   (More details of the conflicts will be shown in future releases)");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n----------------------------------------\r\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"---------- [ Pending Imports ] ---------\r\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"----------------------------------------\r\n");
                        
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
                                    ::SendScintilla(SCI_GOTOLINE,17,0);
                                    ::SendScintilla(SCI_SETSELECTION,0,::SendScintilla(SCI_GETCURRENTPOS,0,0));
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                        
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
                                //Delete the old entry
                                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                                {
                                    sqlite3_bind_text(stmt, 1, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                                    sqlite3_step(stmt);
                                    sqlite3_finalize(stmt);

                                }
                                //write the new entry
                                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
                                {
                                    sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
                                
                                    sqlite3_step(stmt);
                                    sqlite3_finalize(stmt);
                                }

                                //write the old entry back with conflict suffix
                                if (g_dbOpen && SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?)", -1, &stmt, NULL))
                                {
                                    importCount++;
                                    //TODO: add file name to the stamp
                                    char* dateText = getDateTime("yyyyMMdd");
                                    char* timeText = getDateTime("HHmmss",false);
                                    char* tagTextsuffixed;
                                    tagTextsuffixed = new char [strlen(tagText)+256];
                                
                                    strcpy(tagTextsuffixed,tagText);
                                    strcat(tagTextsuffixed,".OldCopy");
                                    strcat(tagTextsuffixed,dateText);
                                    strcat(tagTextsuffixed,timeText);
                                    
                                    sqlite3_bind_text(stmt, 1, tagTextsuffixed, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt, 3, snippetTextOld, -1, SQLITE_STATIC);
                                
                                    sqlite3_step(stmt);


                                    //sqlite3_finalize(stmt);
                                    conflictCount++;
                                    delete [] tagTextsuffixed;
                                    delete [] dateText;
                                    delete [] timeText;
                                }
                            }
                        }
                        delete [] snippetTextOld;
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
            
                ::SendScintilla(SCI_SETSAVEPOINT,0,0);
                updateDockItems(false,false);
            
                ::SendScintilla(SCI_GOTOPOS,0,0);
                next = searchNext("!$[FingerTextData FingerTextData]@#");
            } while (next>=0);
            
            ::SendScintilla(SCI_SETCURSOR, SC_CURSORNORMAL, 0);

            wchar_t importCountText[200] = TEXT("");
            
            if (importCount>1)
            {
                ::_itow_s(importCount, importCountText, 10, 10);
                wcscat_s(importCountText,TEXT(" snippets are imported."));
            } else if (importCount==1)
            {
                wcscat_s(importCountText,TEXT("1 snippet is imported."));
            } else
            {
                wcscat_s(importCountText,TEXT("No Snippets are imported."));
            }

            if (conflictCount>0)
            {
                //TODO: more detail messages and count the number of conflict or problematic snippets
                wcscat_s(importCountText,TEXT("\r\n\r\nThere are some conflicts between the imported and existing snippets. You may go to the snippet editor to clean them up."));
            }
            //::MessageBox(nppData._nppHandle, TEXT("Complete importing snippets"), TEXT("FingerText"), MB_OK);
            ::MessageBox(nppData._nppHandle, importCountText, TEXT("FingerText"), MB_OK);
            
            ::SendScintilla(SCI_SETSAVEPOINT,0,0);
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);

                //updateMode();
                //updateDockItems();
        }
        //delete [] fileText;
    }
    g_liveHintUpdate++;

}

int promptSaveSnippet(TCHAR* message)
{
    int messageReturn = IDNO;
    if (g_editorView)
    {
        //HWND curScintilla = ::getCurrentScintilla();

        if (message == NULL)
        {
            saveSnippet();
        } else if (::SendScintilla(SCI_GETMODIFY,0,0)!=0)
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

void updateLineCount(int count)
{
    if (count>=0)
    {
        g_editorLineCount = count;
    } else
    {
        g_editorLineCount = ::SendScintilla(SCI_GETLINECOUNT,0,0);
    }
}

void updateScintilla()
{
    HWND curScintilla = getCurrentScintilla();
    pSciMsg = (SciFnDirect)SendMessage(curScintilla,SCI_GETDIRECTFUNCTION, 0, 0);
    pSciWndData = (sptr_t)SendMessage(curScintilla,SCI_GETDIRECTPOINTER, 0, 0);
}


void updateMode()
{
    updateScintilla();
    //TODO: should change to edit mode and normal mode by a button, and dynamically adjust the dock content
    //HWND curScintilla = getCurrentScintilla();
    TCHAR fileType[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileType);
        
    if (::_tcscmp(fileType,TEXT("SnippetEditor.ftb"))==0)
    {
        snippetDock.toggleSave(true);
        g_editorView = true;
        snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("EDIT MODE\r\n(Double click item in list to edit another snippet, Ctrl+S to save)"));
        updateLineCount();
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
    if (::MessageBox(nppData._nppHandle, TEXT("Change the settings only when you know what you are doing. Messing up the ini can cause FingerText to stop working.\r\n\r\n Do you wish to continue?"), TEXT("FingerText"), MB_YESNO) == IDYES)
    {
        writeConfig();

        if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_iniPath))
        {
            ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_iniPath);
        }
        //HWND curScintilla = getCurrentScintilla();
        int lineCount = ::SendScintilla(SCI_GETLINECOUNT, 0, 0)-1;
        ::SendScintilla(SCI_ANNOTATIONCLEARALL, 0, 0);
        //TODO: move this part (and other text) to const char in another file.
        ::SendScintilla(SCI_ANNOTATIONSETTEXT, lineCount, (LPARAM)"\
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
 ; live_preview_box           --            0: Turn off preview box live update\r\n\
 ;                                (default) 1: Turn on preview box live update\r\n\
 ; editor_caret_bound         --            0: Fingertext will not restrict caret movement in snippet editing mode.\r\n\
 ;                                             Do not set this to 0 unless you are very sure that you won't mess up\r\n\
 ;                                             the snippet editor format.\r\n\
 ;                            --  (default) 1: Fingertext will restrict caret movement in snippet editing mode.\r\n\
 ; force_multipaste           --            0: Use notepad++ settings to determine whether you can paste text into\r\n\
 ;                                             multiple hotspots simultaneously.\r\n\
 ;                            --  (default) 1: Force notepad++ to turn on multipasting feature.\r\n\
        ");
        ::SendScintilla(SCI_ANNOTATIONSETSTYLE, lineCount, STYLE_INDENTGUIDE);
        ::SendScintilla(SCI_ANNOTATIONSETVISIBLE, lineCount, 0);
    }
}

void showHelp()
{
  TCHAR* helpText = TEXT("\
FingerText Quick Guide:\r\n\r\n\
Insert Snippet --- Type in TriggerText and Hit the tab key\r\n\
Navigate to next Hotspot --- Hit the tab key\r\n\
Show SnippetDock --- Menu>Plugins>FingerText>Toggle On/Off SnippetDock\r\n\
AutoComplete TriggerText --- Menu>Plugins>FingerText>TriggerText Completion\r\n\
Goto Snippet Editor --- Double click a snippet in the SnippetDock\r\n\
Create New Snippet --- Click Create button on the SnippetDock\r\n\
Create snippet from selection -- Select some text and click create button\r\n\
Save Snippet --- In the Snippet Editor View, Click Save Button or Ctrl+S\r\n\
Delete Snippet --- Select a snippet on SnippetDock and Click Delete Button\r\n\
Export Snippets --- Menu>Plugins>FingerText>Export Snippets\r\n\
Delete All Snippets --- Menu>Plugins>FingerText>Export and Delete All Snippets\r\n\
Import Snippets --- Menu>Plugins>FingerText>Import Snippets\r\n\
About FingerText --- Menu>Plugins>FingerText>About\r\n\r\n\
For step by step usage guide, please visit http://github.com/erinata/FingerText \
");

    ::MessageBox(nppData._nppHandle, helpText, TEXT("FingerText"), MB_OK);
     //ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/erinata/FingerText"), NULL, NULL, SW_SHOWNORMAL);
}

void showAbout()
{
    TCHAR versionText[1000];
    _tcscpy_s(versionText,TEXT(""));
    _tcscat_s(versionText,TEXT(VERSION_TEXT_LONG));
    _tcscat_s(versionText,TEXT("\
\r\n\
July 2011\r\n\r\n\
Author: Tom Lam\r\n\
Email: erinata@gmail.com\r\n\r\n\
Update to the lastest version:\r\n\
     http://sourceforge.net/projects/fingertext/ \r\n\
Usage Guide and Source code:\r\n\
     http://github.com/erinata/FingerText \r\n\r\n\
(Snippets created using FingerText 0.3.5 or earlier versions are not compatible with this version)\
"));
    ::MessageBox(nppData._nppHandle, versionText, TEXT("FingerText"), MB_OK);
}

void refreshAnnotation()
{
    if (g_editorView)
    {
        g_selectionMonitor--;
        g_modifyResponse = false;
        TCHAR fileType[MAX_PATH];
        //::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)fileType);
        ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileType);
        
        if (::_tcscmp(fileType,TEXT("SnippetEditor.ftb"))==0)
        {
            //HWND curScintilla = getCurrentScintilla();

            int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION,::SendScintilla(SCI_GETCURRENTPOS,0,0),0);

            //::SendMessage(getCurrentScintilla(), SCI_ANNOTATIONCLEARALL, 0, 0);
            ::SendScintilla(SCI_ANNOTATIONCLEARALL, 0, 0);

            if (lineCurrent == 1)
            {
                ::SendScintilla(SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
__________________________________________________________________________\r\n\
 Snippet Editor Hint: \r\n\r\n\
 Triggertext is the text you type to trigger the snippets.\r\n\
 e.g. \"npp\"(without quotes) means the snippet is triggered \r\n\
 when you type npp and hit tab)\r\n\
__________________________________________________________________________\r\n\r\n\r\n\r\n\
       =============   TriggerText   =============                        ");
            } else if (lineCurrent == 2)
            {
                ::SendScintilla(SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
__________________________________________________________________________\r\n\
 Snippet Editor Hint: \r\n\r\n\
 Scope determines where the snippet is available.\r\n\
 e.g. \"GLOBAL\"(without quotes) for globally available snippets.\r\n\
 \".cpp\"(without quotes) means available in .cpp documents and\r\n\
 \"Lang:HTML\"(without quotes) for all html documents.\r\n\
__________________________________________________________________________\r\n\r\n\r\n\
       =============   TriggerText   =============                        ");
            } else
            {
                ::SendScintilla(SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
__________________________________________________________________________\r\n\
 Snippet Editor Hint: \r\n\r\n\
 Snippet Content is the text that is inserted to the editor when \r\n\
 a snippet is triggered.\r\n\
 It can be as long as many paragraphs or just several words.\r\n\
 Remember to place an [>END<] at the end of the snippet.\r\n\
__________________________________________________________________________\r\n\r\n\r\n\
       =============   TriggerText   =============                        ");
            }

            ::SendScintilla(SCI_ANNOTATIONSETTEXT, 1, (LPARAM)"\r\n       =============      Scope      =============                        ");
            ::SendScintilla(SCI_ANNOTATIONSETTEXT, 2, (LPARAM)"\r\n       ============= Snippet Content =============                        ");
            ::SendScintilla(SCI_ANNOTATIONSETSTYLE, 0, STYLE_INDENTGUIDE);
            ::SendScintilla(SCI_ANNOTATIONSETSTYLE, 1, STYLE_INDENTGUIDE);
            ::SendScintilla(SCI_ANNOTATIONSETSTYLE, 2, STYLE_INDENTGUIDE);
              
            ::SendScintilla(SCI_ANNOTATIONSETVISIBLE, 2, 0);
            
        }
        g_selectionMonitor++;
        g_modifyResponse = true;
    }
}

//For option dynamic hotspot
void cleanOptionItem()
{
   
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
    // TODO: should use stack?
    if (g_optionNumber<20)
    {
        g_optionArray[g_optionNumber] = item;
        g_optionNumber++;
    }
}

//char* getOptionItem()
void updateOptionCurrent(bool toNext)
{
    //char* item;
    //int length = strlen(g_optionArray[g_optionCurrent]);
    //item = new char [length+1];
    //strcpy(item, g_optionArray[g_optionCurrent]);
    if (toNext)
    {
        if (g_optionCurrent >= g_optionNumber-1) 
        {
            g_optionCurrent = 0;
        } else
        {
            g_optionCurrent++;
        }
    } else
    {
        if (g_optionCurrent <= 0) 
        {
            g_optionCurrent = g_optionNumber-1;
        } else
        {
            g_optionCurrent--;
        }
    }
    //return item;
    //return g_optionArray[g_optionCurrent];
}

void turnOffOptionMode()
{
    g_optionMode = false;
}

void optionNavigate(bool toNext)
{
    
    ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    updateOptionCurrent(toNext);
    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);
    ::SendScintilla(SCI_GOTOPOS,g_optionStartPosition,0);
    g_optionEndPosition = g_optionStartPosition + strlen(g_optionArray[g_optionCurrent]);
    ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
}

void selectionMonitor(int contentChange)
{
    //TODO: backspace at the beginning of line 4 in editor mode breaks hint annotation 
    //TODO: pasting text with more then one line in the scope field will break editor restriction
    //TODO: lots of optimization needed
    //In normal view, this code is going to cater the option navigation. In editor view, it restrict selection in first 3 lines
    if (g_selectionMonitor == 1)
    {
        //TODO: this "100" is associated with the limit of number of multiple hotspots that can be simultaneously activated, should find a way to make this more customizable
        if ((::SendScintilla(SCI_GETSELECTIONMODE,0,0)!=SC_SEL_STREAM) || (::SendScintilla(SCI_GETSELECTIONS,0,0)>100))
        {
            g_rectSelection = true;
        } else
        {
            g_rectSelection = false;
        }

        g_modifyResponse = false;
        g_selectionMonitor--;
        if (g_editorView == false)
        {
            
            //TODO: reexamine possible performance improvement
            if (g_optionMode == true)
            {
               
                int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                //alertNumber(g_optionStartPosition);
                //alertNumber(posCurrent);
                //TODO: a bug when there is an empty option and the hotspot is at the beginning of document
                if (posCurrent > g_optionStartPosition)
                {
                    optionNavigate(true);
                    //optionTriggered = true;
                    g_optionMode = true; // TODO: investigate why this line is necessary
                } else
                {
                    optionNavigate(false);
                    g_optionMode = true;
                }
                //else
                //{
                //    cleanOptionItem();
                //    g_optionMode = false;
                //}
            }
        } else if (g_editorCaretBound == 1)
        {
            int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION,posCurrent,0);
            int firstlineEnd = ::SendScintilla(SCI_GETLINEENDPOSITION,0,0);
            int currentLineCount = ::SendScintilla(SCI_GETLINECOUNT,0,0);
            int selectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
            int selectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);
            int selectionStartLine = ::SendScintilla(SCI_LINEFROMPOSITION,selectionStart,0);
            int selectionEndLine = ::SendScintilla(SCI_LINEFROMPOSITION,selectionEnd,0);

            //alertNumber(lineCurrent);
            //if (contentChange)
            if (contentChange & (SC_UPDATE_CONTENT))
            {
                if ((g_editorLineCount < currentLineCount) && (lineCurrent <= 3))
                {
                    //::SendMessage(curScintilla,SCI_UNDO,0,0);
                    ::SendScintilla(SCI_UNDO,0,0);
                    updateLineCount();
                    //updateLineCount(currentLineCount);
                } else if ((g_editorLineCount > currentLineCount) && (lineCurrent <= 2))
                {
                    ::SendScintilla(SCI_UNDO,0,0);
                    updateLineCount();
                    
                } else if (::SendScintilla(SCI_LINELENGTH,1,0)>=41)
                {
                    ::MessageBox(nppData._nppHandle, TEXT("The TriggerText length limit is 40 characters."), TEXT("FingerText"), MB_OK);
                    ::SendScintilla(SCI_UNDO,0,0);
                    updateLineCount();
                
                } else if (::SendScintilla(SCI_LINELENGTH,2,0)>=251)
                {
                    ::MessageBox(nppData._nppHandle, TEXT("The Scope length limit is 250 characters."), TEXT("FingerText"), MB_OK);
                    ::SendScintilla(SCI_UNDO,0,0);
                    updateLineCount();
                } 
            }

            if (lineCurrent <= 0) ::SendScintilla(SCI_GOTOLINE,1,0);

            if ((selectionStartLine != selectionEndLine) && ((selectionStartLine <= 2) || (selectionEndLine <=2)))
            {
                if (selectionEndLine>0)
                {
                    ::SendScintilla(SCI_GOTOLINE,selectionEndLine,0);
                } else
                {
                    ::SendScintilla(SCI_GOTOLINE,1,0);
                }
            }            
            // TODO: a more refine method to adjust the selection when for selection across 2 fields (one method is to adjust the start of selection no matter what, another approach is to look at the distance between the start/end point to the field boundary)
            //
            //if (selectionStartLine != selectionEndLine)
            //{
            //    if ((selectionStartLine <= 2) && (selectionEndLine > 2))
            //    {   
            //        ::SendMessage(curScintilla,SCI_SETSELECTIONSTART,::SendMessage(curScintilla,SCI_POSITIONFROMLINE,selectionEndLine,0),0);
            //    } else if ((selectionStartLine > 2) && (selectionEndLine <= 2))
            //    {
            //        ::SendMessage(curScintilla,SCI_SETSELECTIONEND,::SendMessage(curScintilla,SCI_GETLINEENDPOSITION,selectionStartLine,0),0);
            //    } else if ((selectionStartLine <= 2) && (selectionEndLine <= 2))
            //    {
            //        
            //    }
            //}
            //refreshAnnotation();
        }
        g_modifyResponse = true;
        g_selectionMonitor++;
    }
    
}

void tagComplete()
{
    //TODO: can just use the snippetdock to do the completion (after rewrite of snippetdock)
    int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
    if (triggerTag(posCurrent,true) > 0) snippetHintUpdate();
}

//TODO: better triggertag, should allow for a list of scopes
bool triggerTag(int &posCurrent,bool triggerTextComplete, int triggerLength)
{
    //HWND curScintilla = getCurrentScintilla();
    bool tagFound = false;
    char *tag;
	int tagLength = getCurrentTag(posCurrent, &tag, triggerLength);
    
    //int position = 0;
    //bool groupChecked = false;

    //int curLang = 0;
    //::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE ,0,(LPARAM)&curLang);
    //wchar_t curLangNumber[10];
    //wchar_t curLangText[20];
    //::wcscpy(curLangText, TEXT("LANG_"));
    //::_itow_s(curLang, curLangNumber, 10, 10);
    //::wcscat(curLangText, curLangNumber);

    if (((triggerLength==0) && (tag[0] == '_')) || (tagLength == 0))
    {
        delete [] tag;
    } else if (tagLength > 0) //TODO: changing this to >0 fixed the problem of tag_tab_completion, but need to investigate more about the side effect
	{
        
        int posBeforeTag = posCurrent - tagLength;

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
                ::SendScintilla(SCI_SETSEL,posBeforeTag,posCurrent);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)expanded);
                posBeforeTag = posBeforeTag+strlen(expanded);
            } else
            {
                replaceTag(expanded, posCurrent, posBeforeTag);
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
        if (tagFound) ::SendScintilla(SCI_GOTOPOS,posBeforeTag,0);
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
    char *s[] = {"Lang:TXT","Lang:PHP","Lang:C","Lang:CPP","Lang:CS","Lang:OBJC","Lang:JAVA","Lang:RC",
                 "Lang:HTML","Lang:XML","Lang:MAKEFILE","Lang:PASCAL","Lang:BATCH","Lang:INI","Lang:NFO","",
                 "Lang:ASP","Lang:SQL","Lang:VB","Lang:JS","Lang:CSS","Lang:PERL","Lang:PYTHON","Lang:LUA",
                 "Lang:TEX","Lang:FORTRAN","Lang:BASH","Lang:FLASH","Lang:NSIS","Lang:TCL","Lang:LISP","Lang:SCHEME",
                 "Lang:ASM","Lang:DIFF","Lang:PROPS","Lang:PS","Lang:RUBY","Lang:SMALLTALK","Lang:VHDL","Lang:KIX",
                 "Lang:AU3","Lang:CAML","Lang:ADA","Lang:VERILOG","Lang:MATLAB","Lang:HASKELL","Lang:INNO","",
                 "Lang:CMAKE","Lang:YAML","Lang:COBOL","Lang:GUI4CLI","Lang:D","Lang:POWERSHELL","Lang:R"};
    
    return s[curLang];
    //return "";
}

void tabActivate()
{

    //TODO: in general I should add logo to all the messages
    //HWND curScintilla = getCurrentScintilla();

    //if ((g_enable==false) || (::SendScintilla(SCI_SELECTIONISRECTANGLE,0,0)==1))
    if ((g_enable==false) || (g_rectSelection==true))
    {        
        ::SendScintilla(SCI_TAB,0,0);   
    } else
    {
        int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION,posCurrent,0);

        if ((g_editorView == true) && (lineCurrent <=2))
        {
            if (lineCurrent == 1)
            {
                //TODO: can make the Tab select the whole field instead of just GOTOLINE
                //::SendMessage(curScintilla,SCI_SETSEL,::SendMessage(curScintilla,SCI_POSITIONFROMLINE,2,0),::SendMessage(curScintilla,SCI_GETLINEENDPOSITION,2,0));
                ::SendScintilla(SCI_GOTOLINE,2,0);
            } else if (lineCurrent == 2)
            {
                ::SendScintilla(SCI_GOTOLINE,3,0);
            }

        } else
        {

        //bool optionTriggered = false;
        //if (g_optionMode == true)
        //{
        //    if (posCurrent == g_optionStartPosition)
        //    {
        //        optionNavigate(curScintilla);
        //        optionTriggered = true;
        //        g_optionMode = true; // TODO: investigate why this line is necessary
        //    } else
        //    {
        //        cleanOptionItem();
        //        g_optionMode = false;
        //    }
        //}
        //
        //if (optionTriggered == false)

        //if (g_optionMode == true)
        //{
        //    g_optionMode = false;
        //    ::SendMessage(curScintilla,SCI_GOTOPOS,g_optionEndPosition,0);
        //    snippetHintUpdate();
        //} else
        //{
            g_liveHintUpdate--;
            g_selectionMonitor--;
            int posSelectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
            int posSelectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);
            if (g_preserveSteps==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
            bool tagFound = false;
            if (posSelectionStart==posSelectionEnd)
            {
                tagFound = triggerTag(posCurrent);
            }

            if (tagFound) ::SendScintilla(SCI_AUTOCCANCEL,0,0);
            posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);

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
                dynamicSpot = dynamicHotspot(posCurrent); //TODO: May still consider do some checking before going into dynamic hotspot for performance improvement
                    
                if (g_preserveSteps==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
	            
                //if (specialSpot>=0)
                //{
                ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
                navSpot = hotSpotNavigation();

                if ((navSpot) || (dynamicSpot)) ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                //}
            } else
            {
                if (g_preserveSteps==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);

            }

            bool snippetHint = false;

            bool completeFound = false;
            if (g_tabTagCompletion == 1)
            {
                if ((navSpot == false) && (tagFound == false) && (dynamicSpot==false)) 
	    	    {
                    ::SendScintilla(SCI_GOTOPOS, posCurrent, 0);
                    //completeFound = snippetComplete();
                    completeFound = triggerTag(posCurrent,true);
                    if (completeFound)
                    {
                        ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                        snippetHint = true;
                    }
	    	    }
            }
            
            
            
            if ((navSpot == false) && (tagFound == false) && (completeFound==false) && (dynamicSpot==false)) 
            {
                if (g_optionMode == true)
                {
                    g_optionMode = false;
                    ::SendScintilla(SCI_GOTOPOS,g_optionEndPosition,0);
                    snippetHint = true;
                } else
                {
                    restoreTab(posCurrent, posSelectionStart, posSelectionEnd);
                }
            }

            g_liveHintUpdate++;
            if (snippetHint) snippetHintUpdate();
            g_selectionMonitor++;
        //}
        }
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

void testing2()
{
    ::MessageBox(nppData._nppHandle, TEXT("Testing2!"), TEXT("Trace"), MB_OK);
    //HWND curScintilla = getCurrentScintilla();
}



void testing()
{
    
    ::MessageBox(nppData._nppHandle, TEXT("Testing!"), TEXT("Trace"), MB_OK);
    //HWND curScintilla = getCurrentScintilla();
    alertCharArray("laptop");


    ////Testing creating window using createwindowex
    //WNDCLASSEX wc;
    //HWND hwnd;
    //MSG Msg;
    //
    ////Step 1: Registering the Window Class
    //wc.cbSize        = sizeof(WNDCLASSEX);
    //wc.style         = 0;
    //wc.lpfnWndProc   = WndProc;
    //wc.cbClsExtra    = 0;
    //wc.cbWndExtra    = 0;
    //wc.hInstance     = NULL;
    //wc.hIcon         = LoadIcon(NULL, (LPCWSTR)IDI_APPLICATION);
    //wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    //wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    //wc.lpszMenuName  = NULL;
    //wc.lpszClassName = TEXT("CLASS");
    //wc.hIconSm       = LoadIcon(NULL, (LPCWSTR)IDI_APPLICATION);
    //
    //if(!RegisterClassEx(&wc))
    //{
    //    MessageBox(NULL, TEXT("Window Registration Failed!"), TEXT("Error!"),
    //        MB_ICONEXCLAMATION | MB_OK);
    //    return;
    //}
    //
    //// Step 2: Creating the Window
    //hwnd = CreateWindowEx(
    //    WS_EX_CLIENTEDGE,
    //    TEXT("CLASS"),
    //    TEXT("The title of my window"),
    //    WS_OVERLAPPEDWINDOW,
    //    CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
    //    NULL, NULL, NULL, NULL);
    //
    //if(hwnd == NULL)
    //{
    //    MessageBox(NULL, TEXT("Window Creation Failed!"), TEXT("Error!"),
    //        MB_ICONEXCLAMATION | MB_OK);
    //    return;
    //}
    //
    //ShowWindow(hwnd, 5);
    //UpdateWindow(hwnd);
    //
    //// Step 3: The Message Loop
    //while(GetMessage(&Msg, NULL, 0, 0) > 0)
    //{
    //    TranslateMessage(&Msg);
    //    DispatchMessage(&Msg);
    //}
  
    ////Testing using winhttp to send request
    //Variables 
    //DWORD dwSize = 0;
    //DWORD dwDownloaded = 0;
    //LPSTR pszOutBuffer;
    //std::vector <std::string>  vFileContent;
    //BOOL  bResults = FALSE;
    //HINTERNET  hSession = NULL, 
    //           hConnect = NULL,
    //           hRequest = NULL;
    //
    //// Use WinHttpOpen to obtain a session handle.
    //hSession = WinHttpOpen( L"WinHTTP Example/1.0",  
    //                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    //                        WINHTTP_NO_PROXY_NAME, 
    //                        WINHTTP_NO_PROXY_BYPASS, 0);
    //
    //// Specify an HTTP server.
    //if (hSession)
    //    hConnect = WinHttpConnect( hSession, L"dl.dropbox.com",
    //                               INTERNET_DEFAULT_HTTP_PORT, 0);
    //
    //// Create an HTTP request handle.
    //if (hConnect)
    //    hRequest = WinHttpOpenRequest( hConnect, L"GET", L"/u/7429931/test.txt",
    //                                   NULL, WINHTTP_NO_REFERER, 
    //                                                           NULL, 
    //                                   NULL);
    //
    //// Send a request.
    //if (hRequest)
    //    bResults = WinHttpSendRequest( hRequest,
    //                                   WINHTTP_NO_ADDITIONAL_HEADERS,
    //                                   0, WINHTTP_NO_REQUEST_DATA, 0, 
    //                                   0, 0);
    //
    //
    //// End the request.
    //if (bResults)
    //    bResults = WinHttpReceiveResponse( hRequest, NULL);
    //
    //// Keep checking for data until there is nothing left.
    //if (bResults)
    //    do 
    //    {
    //
    //        // Check for available data.
    //        dwSize = 0;
    //        if (!WinHttpQueryDataAvailable( hRequest, &dwSize))
    //            printf( "Error %u in WinHttpQueryDataAvailable.\n",
    //                    GetLastError());
    //
    //        // Allocate space for the buffer.
    //        pszOutBuffer = new char[dwSize+1];
    //        if (!pszOutBuffer)
    //        {
    //            printf("Out of memory\n");
    //            dwSize=0;
    //        }
    //        else
    //        {
    //            // Read the Data.
    //            ZeroMemory(pszOutBuffer, dwSize+1);
    //
    //            if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, 
    //                                  dwSize, &dwDownloaded))
    //                {
    //                printf( "Error %u in WinHttpReadData.\n", 
    //                        GetLastError());
    //                }
    //            else
    //                {
    //                        printf("%s", pszOutBuffer);
    //                            // Data in vFileContent
    //                        vFileContent.push_back(pszOutBuffer);
    //                }
    //
    //            // Free the memory allocated to the buffer.
    //            delete [] pszOutBuffer;
    //        }
    //
    //    } while (dwSize>0);
    //
    //
    //// Report any errors.
    //if (!bResults)
    //    printf("Error %d has occurred.\n",GetLastError());
    //
    //// Close any open handles.
    //if (hRequest) WinHttpCloseHandle(hRequest);
    //if (hConnect) WinHttpCloseHandle(hConnect);
    //if (hSession) WinHttpCloseHandle(hSession);
    //
    //// Write vFileContent to file
    //std::ofstream out("d:\\test.txt",std::ios::binary);
    //for (int i = 0; i < (int) vFileContent.size();i++)
    //out << vFileContent[i];
    //out.close();

    
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

void alertString(std::string input)
{
    char* temp = new char [input.size()+1];
    strcpy(temp, input.c_str());
    alertCharArray(temp);
    delete [] temp;
}