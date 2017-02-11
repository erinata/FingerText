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

// Notepad++ API stuffs
FuncItem funcItem[MENU_LENGTH];     // The menu item data that Notepad++ needs
NppData nppData;                    // The data for plugin command and sending message to notepad++
HANDLE g_hModule;                   // the hModule from pluginInit for initializing dialogs

HWND g_customSciHandle;

WNDPROC	wndProcNpp = NULL;

// Status dummies 
int nppLoaded = 0;    // Indicates NPP_READY has triggered
int sciFocus = 1;     // Indicates the current focus is on the editor

// Sqlite3
sqlite3 *g_db;                      // For Sqlite3 
bool     g_dbOpen;                  // For Sqlite3 

// Paths
wchar_t g_basePath[MAX_PATH];
TCHAR g_ftbPath[MAX_PATH];
TCHAR g_fttempPath[MAX_PATH];
TCHAR g_currentFocusPath[MAX_PATH];
//TCHAR g_backupPath[MAX_PATH];
TCHAR g_downloadPath[MAX_PATH];


// Config object
PluginConfig pc;

// Dialogs
DockingDlg snippetDock;
InsertionDlg insertionDlg;
SettingDlg settingDlg;
CreationDlg creationDlg;

// Need a record for all the cmdIndex that involve a dock or a shortkey
int g_snippetDockIndex;
int g_tabActivateIndex;
int g_showInsertionDlgIndex;

int g_toggleDisableIndex;
int g_selectionToSnippetIndex;
int g_importSnippetsIndex;
int g_exportSnippetsIndex;
int g_deleteAllSnippetsIndex;
int g_downloadDefaultPackageIndex;
int installDefaultPackageIndex;
int g_installDefaultPackageIndex;
int g_TriggerTextCompletionIndex;
int g_InsertHotspotIndex;
int g_insertPreviousIndex;
int g_settingsIndex;
int g_quickGuideIndex;
int g_aboutIndex;

std::string g_lastTriggerText = "";
std::string g_lastOption = "";
std::string g_lastListItem = "";

bool g_onHotSpot = false;

// For compatibility mode
HHOOK hook = NULL;

struct SnipIndex 
{
    std::string triggerText;
    std::string scope;
    std::string content;    
};
std::vector<SnipIndex> g_snippetCache;

bool g_modifyResponse = true;
int g_selectionMonitor = 1;
bool g_rectSelection = false;
bool g_freezeDock = false;
bool g_enable = true;
bool g_editorView;

int g_editorLineCount;

std::string g_snippetCount = "";

bool g_fingerTextList;

int g_lastTriggerPosition = 0;
std::string g_customClipBoard = "";
std::string g_selectedText = "";

// For option hotspot
bool g_optionMode = false;
int g_optionStartPosition = 0;
int g_optionEndPosition = 0;
int g_optionCurrent = 0;
std::vector<std::string> g_optionArray;

// List of acceptable tagSigns
char *g_tagSignList[] = {"$[0[","$[![","$[1[","$[2[","$[3["};
char *g_tagTailList[] = {"]0]","]!]","]1]","]2]","]3]"};
char* g_stopCharArray;
int g_listLength = 5;

//For params insertion
std::vector<std::string> g_hotspotParams;

//support the languages supported by npp 0.5.9, excluding "user defined language" abd "search results"
const std::string langList[] = {"TXT","PHP","C","CPP","CS","OBJC","JAVA","RC",
                                "HTML","XML","MAKEFILE","PASCAL","BATCH","INI","NFO","",
                                "ASP","SQL","VB","JS","CSS","PERL","PYTHON","LUA",
                                "TEX","FORTRAN","BASH","FLASH","NSIS","TCL","LISP","SCHEME",
                                "ASM","DIFF","PROPS","PS","RUBY","SMALLTALK","VHDL","KIX",
                                "AU3","CAML","ADA","VERILOG","MATLAB","HASKELL","INNO","",
                                "CMAKE","YAML","COBOL","GUI4CLI","D","POWERSHELL","R"};

// The word char settings for scope and triggertext
const char* scopeWordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:.|";
const char* triggertextWordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-^";
const char* searchWordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-";
char escapeWordChar[200];

//For SETWIN
HWND g_tempWindowHandle;
wchar_t* g_tempWindowKey;

//TODO: Add icon to messageboxes

// Initialize your plugin data here. Called while plugin loading. This runs before setinfo.
void pluginInit(HANDLE hModule)
{
    g_hModule = hModule;  // For dialogs initialization
}

void dialogsInit()
{
    snippetDock.init((HINSTANCE)g_hModule, nppData._nppHandle);
    insertionDlg.init((HINSTANCE)g_hModule, nppData);
    settingDlg.init((HINSTANCE)g_hModule, nppData);
    creationDlg.init((HINSTANCE)g_hModule, nppData);
}

void pathInit()
{
    // Get the config folder of notepad++ and append the plugin name to form the root of all config files
    ::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, reinterpret_cast<LPARAM>(g_basePath));
    ::_tcscat_s(g_basePath,TEXT("\\"));
    ::_tcscat_s(g_basePath,TEXT(PLUGIN_NAME));
    if (!PathFileExists(g_basePath)) ::CreateDirectory(g_basePath, NULL);
    
    // Initialize the files needed (ini and database paths are initalized in configInit and databaseInit)
    ::_tcscpy_s(g_fttempPath,g_basePath);
    ::_tcscat_s(g_fttempPath,TEXT("\\"));
    ::_tcscat_s(g_fttempPath,TEXT(PLUGIN_NAME));
    ::_tcscat_s(g_fttempPath,TEXT(".fttemp"));
    if (!PathFileExists(g_fttempPath)) emptyFile(g_fttempPath);

    ::_tcscpy_s(g_ftbPath,g_basePath);
    ::_tcscat_s(g_ftbPath,TEXT("\\SnippetEditor.ftb"));
    if (!PathFileExists(g_ftbPath)) emptyFile(g_ftbPath);

    //::_tcscpy_s(g_backupPath,g_basePath);
    //::_tcscat_s(g_backupPath,TEXT("\\SnippetsBackup.ftd"));
    //if (!PathFileExists(g_backupPath)) emptyFile(g_backupPath);

    ::_tcscpy_s(g_downloadPath,g_basePath);
    ::_tcscat_s(g_downloadPath,TEXT("\\SnippetsDownloaded.ftd"));
    if (!PathFileExists(g_downloadPath)) emptyFile(g_downloadPath);
    
}

void configInit()
{
    ::_tcscpy_s(pc.iniPath,g_basePath);
    ::_tcscat_s(pc.iniPath,TEXT("\\"));
    ::_tcscat_s(pc.iniPath,TEXT(PLUGIN_NAME));
    ::_tcscat_s(pc.iniPath,TEXT(".ini"));
    if (PathFileExists(pc.iniPath) == false) emptyFile(pc.iniPath);

    pc.configSetUp();
}


void dataBaseInit()
{
    char* dataBasePath = new char[MAX_PATH];
    char* basePath = toCharArray(g_basePath);
    strcpy(dataBasePath,basePath);
    strcat(dataBasePath,"\\");
    strcat(dataBasePath,PLUGIN_NAME);
    strcat(dataBasePath,".db3");
    delete [] basePath;

    bool dbError = sqlite3_open(dataBasePath, &g_db);

    if (dbError)
    {
        g_dbOpen = false;
        showMessageBox(TEXT("Cannot find or open database file in config folder"));
    } else
    {
        g_dbOpen = true;
    }

    delete [] dataBasePath; 
    if (!g_dbOpen) return;

    sqlite3_stmt *stmt;

    if (SQLITE_OK == sqlite3_prepare_v2(g_db, 
    "CREATE TABLE snippets (tag TEXT, tagType TEXT, snippet TEXT, package TEXT)"
    , -1, &stmt, NULL))
    {
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);

    //TODO: a checking on new update can be done by calling pc.newUpdate == false
    // for those who upgrade from old database 
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, 
    "ALTER TABLE snippets ADD COLUMN package TEXT"
    , -1, &stmt, NULL))
    {
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    

    //alert(pc.version);
    //alert(pc.versionOld);
}

// Initialization of plugin commands
void commandMenuInit()
{    
    ShortcutKey *shKey;
    TCHAR* tabActivateText;
    if (!(pc.configInt[USE_NPP_SHORTKEY]))
    {
        shKey = NULL;
        tabActivateText = TEXT("Hotkey remapping disabled (use TAB to trigger snippet)");
    } else
    {
        shKey = setShortCutKey(false,false,false,VK_TAB);
        tabActivateText = TEXT("Trigger Snippet/Navigate to Hotspot");
    }

    ShortcutKey *shKey2;
    shKey2 = setShortCutKey(true,false,false,VK_OEM_2);
    //shKey2 = setShortCutKey(true,false,false,190);
    
    g_tabActivateIndex = setCommand(tabActivateText, tabActivate, shKey);
    setCommand();
    g_snippetDockIndex = setCommand(TEXT("Toggle On/off SnippetDock"), showSnippetDock);
    g_showInsertionDlgIndex = setCommand(TEXT("Show Snippet Insertion Dialog"), showInsertionDlg,shKey2);
    g_toggleDisableIndex = setCommand(TEXT("Toggle On/Off FingerText"), toggleDisable);
    setCommand();
    g_selectionToSnippetIndex = setCommand(TEXT("Create Snippet from Selection"), doSelectionToSnippet);
    g_downloadDefaultPackageIndex = setCommand(TEXT("Install Default Snippet Package"), installDefaultPackage);
    g_importSnippetsIndex = setCommand(TEXT("Import Snippets from ftd file"), importSnippetsOnly);
    //g_downloadDefaultPackageIndex = setCommand(TEXT("Import Default Snippet Package"), downloadDefaultPackage);
    
    g_exportSnippetsIndex = setCommand(TEXT("Export All Snippets"), exportSnippetsOnly);
    g_deleteAllSnippetsIndex = setCommand(TEXT("Delete All Snippets"), exportAndClearSnippets);
    
    setCommand();
    g_TriggerTextCompletionIndex = setCommand(TEXT("TriggerText Completion"), doTagComplete);
    g_insertPreviousIndex = setCommand(TEXT("Insert Previous Snippet"), insertPrevious);
    //g_InsertHotspotIndex =setCommand(TEXT("Insert a hotspot"), insertHotSpotSign);
    setCommand();
    g_settingsIndex = setCommand(TEXT("Settings"), showSettings);
    
    g_quickGuideIndex = setCommand(TEXT("Quick Guide"), showHelp);
    g_aboutIndex = setCommand(TEXT("About"), showAbout);
    setCommand();
    setCommand(TEXT("Testing"), testing);
    setCommand(TEXT("Testing2"), testing2);
    setCommand(TEXT("Test Settings"), showSettingDlg);
    setCommand(TEXT("Test Creation"), showCreationDlg);


}

void variablesInit()
{
    g_stopCharArray = new char[strlen(g_tagSignList[0])+strlen(g_tagTailList[0])+1];
    strcpy(g_stopCharArray,g_tagSignList[0]);
    strcat(g_stopCharArray,g_tagTailList[0]);

    strcpy(escapeWordChar,searchWordChar);
    if (wcslen(pc.configText[CUSTOM_ESCAPE_CHAR])>0)
    {
        char *customEscapeChar = toCharArray(pc.configText[CUSTOM_ESCAPE_CHAR]);
        strncat(escapeWordChar,customEscapeChar,20);
        delete [] customEscapeChar;
    }

    updateSnippetCount();
    g_customSciHandle = (HWND)::SendMessage(nppData._nppHandle,NPPM_CREATESCINTILLAHANDLE,0,NULL); 


}

void nppReady()
{
    sciFocus = 1;
    if (g_dbOpen)
    {
        g_enable = true;
    } else 
    {
        g_enable = false; 
        showMessageBox(TEXT("FingerText cannot be enabled because there is no database connection. Please restart Notepad++ and make sure that the config folder is writable."));
    }

    turnOffOptionMode();
    if (!(pc.configInt[USE_NPP_SHORTKEY]))                                                         // For compatibility mode
    {                                                                                              // For compatibility mode
        ::EnableMenuItem((HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0), funcItem[g_tabActivateIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);    // For compatibility mode
        installhook();                                                                             // For compatibility mode
        SendScintilla(SCI_ASSIGNCMDKEY,SCK_TAB,SCI_NULL);                                          // For compatibility mode
    }                                                                                              // For compatibility mode

    //Temporarily disable the insertion dialog
    //::EnableMenuItem((HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0), funcItem[g_showInsertionDlgIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);

    updateMode();
    pc.upgradeMessage();

    if (pc.configInt[FORCE_MULTI_PASTE]) ::SendScintilla(SCI_SETMULTIPASTE,1,0); 

    if (snippetDock.isVisible()) updateDockItems(true,false,"%",true); //snippetHintUpdate();
}

void pluginShutdown()  // function is triggered when NPPN_SHUTDOWN fires  
{   
    ::SendMessage(nppData._nppHandle,NPPM_DESTROYSCINTILLAHANDLE,0,(LPARAM)g_customSciHandle);

    if (!(pc.configInt[USE_NPP_SHORTKEY])) removehook();  // For compatibility mode
    //delete [] g_snippetCache;
    
    if (g_dbOpen)
    {
        sqlite3_close(g_db);  // This close the database when the plugin shutdown.
        g_dbOpen = false;
    }

    ::SetWindowLongPtr(nppData._nppHandle, GWLP_WNDPROC, (LPARAM)wndProcNpp); // Clean up subclassing
    
    delete [] g_stopCharArray;

    pc.configCleanUp();
    
}

// command shortcut clean up
void commandMenuCleanUp()
{
    delete funcItem[g_tabActivateIndex]._pShKey;
    delete funcItem[g_showInsertionDlgIndex]._pShKey;
    
	// Don't forget to deallocate your shortcut here
}

void pluginCleanUp()
{
    //TODO: think about how to save the parameters for the next session during clean up    
}

// Functions for Fingertext

void shortCutRemapped()
{
    if (!(pc.configInt[USE_NPP_SHORTKEY]))
    {
        HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0);             // For compatibility mode
        ::EnableMenuItem(hMenu, funcItem[g_tabActivateIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);       // For compatibility mode
    }
}


void toggleDisable()
{
    HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0);             // For compatibility mode
        ::EnableMenuItem(hMenu, funcItem[g_tabActivateIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
    if (g_enable)
    {
        ::EnableMenuItem(hMenu, funcItem[g_snippetDockIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_tabActivateIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_showInsertionDlgIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        
        ::EnableMenuItem(hMenu, funcItem[g_selectionToSnippetIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_importSnippetsIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_exportSnippetsIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_deleteAllSnippetsIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_TriggerTextCompletionIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        ::EnableMenuItem(hMenu, funcItem[g_InsertHotspotIndex]._cmdID, MF_BYCOMMAND | MF_GRAYED);
        
        closeEditor();
        snippetDock.display(false);
        // TODO: refactor all the message boxes to a separate function
        showMessageBox(TEXT("FingerText is disabled"));
        
        //::MessageBox(nppData._nppHandle, TEXT("FingerText is disabled"), TEXT(PLUGIN_NAME), MB_OK);
        g_enable = false;
    } else if (!g_dbOpen)
    {
        showMessageBox(TEXT("FingerText cannot be enabled because there is no database connection. Please restart Notepad++ and make sure that the config folder is writable."));
    } else
    {

        ::EnableMenuItem(hMenu, funcItem[g_snippetDockIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_tabActivateIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_showInsertionDlgIndex]._cmdID, MF_BYCOMMAND);
        
        ::EnableMenuItem(hMenu, funcItem[g_selectionToSnippetIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_importSnippetsIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_exportSnippetsIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_deleteAllSnippetsIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_TriggerTextCompletionIndex]._cmdID, MF_BYCOMMAND);
        ::EnableMenuItem(hMenu, funcItem[g_InsertHotspotIndex]._cmdID, MF_BYCOMMAND);

        showMessageBox(TEXT("FingerText is enabled"));
        //::MessageBox(nppData._nppHandle, TEXT("FingerText is enabled"), TEXT(PLUGIN_NAME), MB_OK);
        g_enable = true;
    }

    updateMode();
}


char *findTagSQLite(char *tag, const char *tagCompare)
{
    //alertCharArray(tagCompare);
	char *expanded = NULL;
	sqlite3_stmt *stmt;

    // First create the SQLite SQL statement ("prepare" it for running)
    char *sqlitePrepareStatement;
    
    sqlitePrepareStatement = "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ? ORDER BY tag";
        
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, sqlitePrepareStatement, -1, &stmt, NULL))
	{
        sqlite3_bind_text(stmt, 1, tagCompare, -1, SQLITE_STATIC);

	    sqlite3_bind_text(stmt, 2, tag, -1, SQLITE_STATIC);

		// Run the query with sqlite3_step
		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
		{
			const char* expandedSQL = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
			//expanded = new char[strlen(expandedSQL)*4 + 1];
            expanded = new char[strlen(expandedSQL) + 1];
			strcpy(expanded, expandedSQL);
		}
	}
    // Close the SQLite statement, as we don't need it anymore
	// This also has the effect of free'ing the result from sqlite3_column_text 
	// (i.e. in our case, expandedSQL)
	sqlite3_finalize(stmt);
	return expanded; //remember to delete the returned expanded after use.
}

void doSelectionToSnippet()
{
    selectionToSnippet(false);
}

void selectionToSnippet(bool forceNew)
{
    g_selectionMonitor--;
    
    //pc.configInt[EDITOR_CARET_BOUND]--;
    
    //HWND curScintilla = getCurrentScintilla();
    int selectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);
    int selectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
    bool withSelection = false;

    char* selection;
    
    if ((selectionEnd>selectionStart) && (!forceNew))
    {
        
        sciGetText(&selection,selectionStart,selectionEnd);
        //selection = new char [selectionEnd - selectionStart +1];
        //::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
        withSelection = true;
    } else
    {
        selection = "This is some stub text for the content of your new snippet.\r\nPlease replace the stub text with the content that you want to show when the snippet is triggered.\r\n\r\nEnjoy!\r\n";
    }
    
    //::SendMessage(curScintilla,SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(selection));
    openTab(g_ftbPath);
    //if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
    //{
    //    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_ftbPath);
    //} 
    //curScintilla = getCurrentScintilla();
    
    //TODO: consider using YES NO CANCEL dialog in promptsavesnippet
    promptSaveSnippet(TEXT("Do you wish to save the current snippet before creating a new one?"));
    
    ::SendScintilla(SCI_CONVERTEOLS,SC_EOL_LF, 0);
    
    ::SendScintilla(SCI_CLEARALL,0,0);
    ::SendScintilla(SCI_INSERTTEXT,::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"------ FingerText Snippet Editor View ------\r\n");
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
    
    //pc.configInt[EDITOR_CARET_BOUND]++;
    g_selectionMonitor++;
}

void closeNonSessionTabs()
{
    closeTab(g_ftbPath);
    closeTab(pc.iniPath);
}

void closeEditor()
{
    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
}

void insertSnippet()
{
    TCHAR* bufferWide;
    snippetDock.getSelectText(bufferWide);
    char* buffer = toCharArray(bufferWide);
    buffer = quickStrip(buffer, ' ');

    int scopeLength = ::strchr(buffer,'>') - buffer - 1;
    int triggerTextLength = strlen(buffer)-scopeLength - 2;
    char* tempTriggerText = new char [ triggerTextLength+1];
    char* tempScope = new char[scopeLength+1];
    
    strncpy(tempScope,buffer+1,scopeLength);
    tempScope[scopeLength] = '\0';
    strncpy(tempTriggerText,buffer+1+scopeLength+1,triggerTextLength);
    tempTriggerText[triggerTextLength] = '\0';
    
    delete [] buffer;

    diagActivate(tempTriggerText);
    ::SetFocus(::getCurrentScintilla());

    
}


void editSnippet()
{
    int topIndex = -1;
    if (g_editorView) topIndex = snippetDock.getTopIndex();

    TCHAR* bufferWide;
    snippetDock.getSelectText(bufferWide);
    char* buffer = toCharArray(bufferWide);
    buffer = quickStrip(buffer, ' ');

    if (strlen(buffer)==0) selectionToSnippet(true);
    //if (strlen(buffer)==0)
    //{
    //    ::showMessageBox(TEXT("No Snippet Selected"));
    //    delete [] buffer;
    //    return;
    //}
    //
    int scopeLength = ::strchr(buffer,'>') - buffer - 1;
    int triggerTextLength = strlen(buffer)-scopeLength - 2;
    char* tempTriggerText = new char [ triggerTextLength+1];
    char* tempScope = new char[scopeLength+1];
    
    strncpy(tempScope,buffer+1,scopeLength);
    tempScope[scopeLength] = '\0';
    strncpy(tempTriggerText,buffer+1+scopeLength+1,triggerTextLength);
    tempTriggerText[triggerTextLength] = '\0';
    
    delete [] buffer;

    sqlite3_stmt *stmt;
    
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType = ? AND tag = ?", -1, &stmt, NULL))
	{
		// Then bind the two ? parameters in the SQLite SQL to the real parameter values
		sqlite3_bind_text(stmt, 1, tempScope , -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, tempTriggerText, -1, SQLITE_STATIC);
		// Run the query with sqlite3_step
		if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
		{
			const char* snippetText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
    
            // After loading the content, switch to the editor buffer and promput for saving if needed
            openTab(g_ftbPath);

            std::string allScope = "";
            //if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
            //{
            //    ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)g_ftbPath);
            //}
            //HWND curScintilla = getCurrentScintilla();
            promptSaveSnippet(TEXT("Do you wish to save the current snippet before editing another one?"));
            
            ::SendScintilla(SCI_CONVERTEOLS,SC_EOL_LF, 0);
            
            sqlite3_stmt *stmt2;
            if (SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT tagtype FROM snippets WHERE tag = ? AND snippet = ?", -1, &stmt2, NULL))
            {
                sqlite3_bind_text(stmt2, 1, tempTriggerText , -1, SQLITE_STATIC);
		        sqlite3_bind_text(stmt2, 2, snippetText, -1, SQLITE_STATIC);

                while (SQLITE_ROW == sqlite3_step(stmt2))
                {
                    if (allScope.length()!=0) allScope = allScope + "|";
                    const char* extraScope = reinterpret_cast<const char *>(sqlite3_column_text(stmt2, 0));
                    allScope = allScope + extraScope;

                }

            }
            sqlite3_finalize(stmt2);

            ::SendScintilla(SCI_CLEARALL,0,0);
            //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"------ FingerText Snippet Editor View ------\r\n");
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)tempTriggerText);
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"\r\n");
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)allScope.c_str());
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)"\r\n");
    
            ::SendScintilla(SCI_INSERTTEXT, ::SendScintilla(SCI_GETLENGTH,0,0), (LPARAM)snippetText);
    
            g_editorView = true;
            refreshAnnotation();
		}
	}
    
	sqlite3_finalize(stmt);
    
    ::SendScintilla(SCI_SETSAVEPOINT,0,0);
    ::SendScintilla(SCI_EMPTYUNDOBUFFER,0,0);
    delete [] tempTriggerText;
    delete [] tempScope;

    int scrollPos = snippetDock.searchSnippetList(bufferWide);
    snippetDock.selectSnippetList(scrollPos);
    if (topIndex == -1)
    {
        snippetDock.setTopIndex(scrollPos);
    } else
    {
        snippetDock.setTopIndex(topIndex);
    }


    delete [] bufferWide;
}

void deleteSnippet()
{
    int topIndex = snippetDock.getTopIndex();
    
    TCHAR* bufferWide;
    snippetDock.getSelectText(bufferWide);
    char* buffer = toCharArray(bufferWide);
    buffer = quickStrip(buffer, ' ');

    int scopeLength = ::strchr(buffer,'>') - buffer - 1;
    int triggerTextLength = strlen(buffer)-scopeLength - 2;
    char* tempTriggerText = new char [ triggerTextLength+1];
    char* tempScope = new char[scopeLength+1];
    
    strncpy(tempScope,buffer+1,scopeLength);
    tempScope[scopeLength] = '\0';
    strncpy(tempTriggerText,buffer+1+scopeLength+1,triggerTextLength);
    tempTriggerText[triggerTextLength] = '\0';
    
    delete [] buffer;

    sqlite3_stmt *stmt;
    
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
    {
        sqlite3_bind_text(stmt, 1, tempScope, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, tempTriggerText, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    //TODO: can use the sqlite3 return message to show error message when the delete is not successful

    updateSnippetCount();
    updateDockItems(true,false,"%",true);

    delete [] tempTriggerText;
    delete [] tempScope;


    snippetDock.setTopIndex(topIndex);
    
    delete [] bufferWide;
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

        //char* wordChar;
        //if (lineNumber==2)
        //{
        //    wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_:.|";
        //    
        //} else //if (lineNumber==1)
        //{
        //    wordChar = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
        //}

        if (lineNumber==2)
        {
            ::SendScintilla(SCI_SETWORDCHARS, 0, (LPARAM)scopeWordChar);
        } else
        {
            ::SendScintilla(SCI_SETWORDCHARS, 0, (LPARAM)triggertextWordChar);
        }
        tagPosEnd = ::SendScintilla(SCI_WORDENDPOSITION,tagPosStart,0);
        ::SendScintilla(SCI_SETCHARSDEFAULT, 0, 0);
        //::SendMessage(curScintilla,SCI_SEARCHANCHOR,0,0);
        //::SendMessage(curScintilla,SCI_SEARCHNEXT,0,(LPARAM)" ");
        //tagPosEnd = ::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0);
        if ((tagPosEnd>tagPosLineEnd) || (tagPosEnd-tagPosStart<=0))
        {
            //blank
            ::SendScintilla(SCI_GOTOLINE,lineNumber,0);
            showMessageBox(errorText);
            //::MessageBox(nppData._nppHandle, errorText, TEXT(PLUGIN_NAME), MB_OK);
            problemSnippet = true;
            
        } else if (tagPosEnd<tagPosLineEnd)
        {
            // multi
            ::SendScintilla(SCI_GOTOLINE,lineNumber,0);
            showMessageBox(errorText);
            //::MessageBox(nppData._nppHandle, errorText, TEXT(PLUGIN_NAME), MB_OK);
            problemSnippet = true;
        }
    }

    if (lineNumber == 3)
    {
        ::SendScintilla(SCI_GOTOPOS,tagPosStart,0);
        int spot = searchNext("[>END<]");
        if (spot<0)
        {
            showMessageBox(TEXT("You should put an \"[>END<]\" (without quotes) at the end of your snippet content."));
            //::MessageBox(nppData._nppHandle, TEXT("You should put an \"[>END<]\" (without quotes) at the end of your snippet content."), TEXT(PLUGIN_NAME), MB_OK);
            problemSnippet = true;
        }
    }

    //::SendScintilla(SCI_SETSELECTION,tagPosStart,tagPosEnd);
    //*buffer = new char[tagPosEnd-tagPosStart + 1];
    //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*buffer));

    sciGetText(&*buffer,tagPosStart,tagPosEnd);

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

    ::SendScintilla(SCI_CONVERTEOLS,SC_EOL_LF, 0);

    if (getLineChecked(&tagText,1,TEXT("TriggerText cannot be blank, and it can only contain alphanumeric characters (no spaces allowed)"))==true) problemSnippet = true;
    if (getLineChecked(&tagTypeText,2,TEXT("Scope cannot be blank, and it can only contain alphanumeric characters and/or period."))==true) problemSnippet = true;
    if (getLineChecked(&snippetText,3,TEXT("Snippet Content cannot be blank."))==true) problemSnippet = true;
    
    ::SendScintilla(SCI_SETSELECTION,docLength,docLength+1); //Take away the extra space added
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");

    std::vector<std::string> tagTypeTextVector = toVectorString(tagTypeText,'|');

    if (!problemSnippet)
    {
        int i = 0;
        while (i<tagTypeTextVector.size())
        {
            if (tagTypeTextVector[i].length()>0)
            {
                // checking for existing snippet 
                sqlite3_stmt *stmt;

                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
                {
                    sqlite3_bind_text(stmt, 1, tagTypeTextVector[i].c_str(), -1, SQLITE_STATIC);
		            sqlite3_bind_text(stmt, 2, tagText, -1, SQLITE_STATIC);
                    if(SQLITE_ROW == sqlite3_step(stmt))
                    {
                        std::string messageString = "Snippet \"" + std::string(tagText) + "\" in scope <" + tagTypeTextVector[i] + "> aleady exists, overwrite?";
                        //char* message = new char[messageString.length()+1];
                        //strcpy(message,messageString.c_str());
                        wchar_t* messageWide = toWideChar((std::string)messageString);
                        int messageReturn = showMessageBox(messageWide,MB_YESNO);
                        //delete [] message;
                        delete [] messageWide;
                        //int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("Snippet exists, overwrite?"), TEXT(PLUGIN_NAME), MB_YESNO);
                        if (messageReturn==IDNO)
                        {
                            
                            // not overwrite
                            std::string messageString = "Snippet \"" + std::string(tagText) + "\" in scope <" + tagTypeTextVector[i] + "> is not saved.";
                            wchar_t* messageWide = toWideChar((std::string)messageString);

                            showMessageBox(messageWide);
                            delete [] messageWide;

                            delete [] tagText;
                            delete [] tagTypeText;
                            delete [] snippetText;
                            //::MessageBox(nppData._nppHandle, TEXT("The Snippet is not saved."), TEXT(PLUGIN_NAME), MB_OK);
                            //::SendMessage(curScintilla, SCI_GOTOPOS, 0, 0);
                            //::SendMessage(curScintilla, SCI_INSERTTEXT, 0, (LPARAM)" ");
                            ::SendScintilla(SCI_SETSELECTION, 0, 1);
                            ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"-");
                            ::SendScintilla(SCI_GOTOPOS, 0, 0);
                            sqlite3_finalize(stmt);
                            return;

                        } else
                        {
                            sqlite3_stmt *stmt2;
                            // delete existing entry
                            if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt2, NULL))
                            {
                                sqlite3_bind_text(stmt2, 1, tagTypeTextVector[i].c_str(), -1, SQLITE_STATIC);
		                        sqlite3_bind_text(stmt2, 2, tagText, -1, SQLITE_STATIC);
                                sqlite3_step(stmt2);
                            
                            } else
                            {
                                showMessageBox(TEXT("Cannot write into database."));
                                //::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT(PLUGIN_NAME), MB_OK);
                            }
                            sqlite3_finalize(stmt2);
                        
                        }
                    }
                }
                sqlite3_finalize(stmt);

                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?,?)", -1, &stmt, NULL))
	            {
		            // Then bind the two ? parameters in the SQLite SQL to the real parameter values
		            sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
		            sqlite3_bind_text(stmt, 2, tagTypeTextVector[i].c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 4, "", -1, SQLITE_STATIC);
    
		            // Run the query with sqlite3_step
		            sqlite3_step(stmt); // SQLITE_ROW 100 sqlite3_step() has another row ready

                    std::string messageString = "Snippet \"" + std::string(tagText) + "\" in scope <" + tagTypeTextVector[i] + "> is saved.";
                    wchar_t* messageWide = toWideChar((std::string)messageString);

                    showMessageBox(messageWide);
                    delete [] messageWide;
                    //showMessageBox(TEXT("The Snippet is saved."));
                    //::MessageBox(nppData._nppHandle, TEXT("The Snippet is saved."), TEXT(PLUGIN_NAME), MB_OK);
	            }
                sqlite3_finalize(stmt);
            }
            i++;
        }

        ::SendScintilla(SCI_SETSAVEPOINT,0,0);
    }

    updateSnippetCount();

    updateDockItems(true,false,"%",true);

    //TODO: This is not working. The scrolling works but the snippetdock reset the scrolling after thei savesnippet() finished   
    wchar_t* searchItem = constructDockItems(toString(tagTypeText),toString(tagText),14);
    int scrollPos = snippetDock.searchSnippetList(searchItem);
    snippetDock.selectSnippetList(scrollPos);
    snippetDock.setTopIndex(scrollPos);
    
    delete [] searchItem;
    delete [] tagText;
    delete [] tagTypeText;
    delete [] snippetText;
    
    g_selectionMonitor++;

}

void restoreTab(int &posCurrent, int &posSelectionStart, int &posSelectionEnd)
{
    // restoring the original tab action
    ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
    ::SendScintilla(SCI_SETSELECTION,posSelectionStart,posSelectionEnd);
    ::SendScintilla(SCI_TAB,0,0);	
}

//TODO: refactor searchPrevMatchedSign and searchNextMatchedTail
int searchPrevMatchedSign(char* tagSign, char* tagTail)
{
    //This function works when the caret is at the beginning of tagtail
    // it return the position at the beginning of the tagsign if found
    int signSpot = -1;
    int tailSpot = -1;
    int unmatchedTail = 0;
    
    do
    {
        
        int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        tailSpot = searchPrev(tagTail);
        ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
        signSpot = searchPrev(tagSign);
        if (signSpot == -1) 
        {
            
            return -1;
        }

        if ((signSpot > tailSpot) && (unmatchedTail == 0))
        {
            
            return signSpot;
        } else if (signSpot > tailSpot)
        {
            ::SendScintilla(SCI_GOTOPOS,signSpot,0);
            unmatchedTail--;
        } else
        {
            ::SendScintilla(SCI_GOTOPOS,tailSpot,0);
            unmatchedTail++;
        
        }
        
    } while (1);
    return -1;
}

int searchNextMatchedTail(char* tagSign, char* tagTail)
{

    // TODO: this function is not returning the position correctly, but it stop at the place where is find the tail
    // This function is tested to work when the position is at the end of tagSign
    // And this return the position at the END of tailsign, if found
   
    int signSpot = -1;
    int tailSpot = -1;
    int unmatchedSign = 0;
    
    int signLength = strlen(tagSign);
    int tailLength = strlen(tagTail);

    do
    {
        int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        signSpot = searchNext(tagSign);
        if (signSpot != -1) signSpot = signSpot+signLength;
        ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
        tailSpot = searchNext(tagTail);
        if (tailSpot != -1) tailSpot = tailSpot+tailLength;
        
        //alertNumber(tailSpot);
        //alertNumber(signSpot);
        //alertNumber(unmatchedSign); 
        
        if (tailSpot == -1) return -1;

        if (((signSpot > tailSpot) || ((signSpot == -1)  && (tailSpot>=0))) && (unmatchedSign == 0))
        {
            return tailSpot;
        } else if (((signSpot > tailSpot) || ((signSpot == -1)  && (tailSpot>=0))))
        {
            ::SendScintilla(SCI_GOTOPOS,tailSpot,0);
            unmatchedSign--;
        } else
        {
            ::SendScintilla(SCI_GOTOPOS,signSpot,0);
            unmatchedSign++;
        }
        
    } while (1);
    return -1;
}

bool dynamicHotspot(int &startingPos, char* tagSign, char* tagTail)
{
    
    int checkPoint = startingPos;    
    bool normalSpotTriggered = false;
    
    //char tagSign[] = "$[![";
    //int tagSignLength = strlen(tagSign);
    //char tagTail[] = "]!]";
    //int tagTailLength = strlen(tagTail);

    //int tagSignLength = strlen(tagSign);
    int tagSignLength = 4;

    char* hotSpotText = 0;
    char* hotSpot = 0;
    int spot = -1;
    int spotComplete = -1;
    int spotType = 0;

    int limitCounter = 0;
    do 
    {
        
        //alertString(g_hotspotParams[0]);
        ::SendScintilla(SCI_GOTOPOS,checkPoint,0);
        spot = searchNext(tagTail);   // Find the tail first so that nested snippets are triggered correctly
        //spot = searchNext(curScintilla, tagSign);
        
        if (spot>=0)
	    {
            //int tailPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            checkPoint = spot+1;
            spotComplete = -1;
            //spotComplete = searchPrev(tagSign);
            
            spotComplete = searchPrevMatchedSign(tagSign,tagTail);
            
            if (spotComplete>=0)
            {

                int firstPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                int secondPos = 0;
                
                spotType = grabHotSpotContent(&hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength,spot);
                
                if (spotType>0)
                {

                    if (!normalSpotTriggered)
                    {
                        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)hotSpotText+5);
                 
                        ::SendScintilla(SCI_GOTOPOS,secondPos+3,0); // TODO: Check whether this GOTOPOS is necessary
                
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
                        } else if (spotType == 5)
                        {
                            checkPoint = firstPos;
                            evaluateHotSpot(firstPos,hotSpotText+5);

                            limitCounter++;
                        } else if (spotType == 6)
                        {
                            checkPoint = firstPos;
                            webRequest(firstPos,hotSpotText+5);

                            limitCounter++;
                        }
                    } else
                    {
                        //alert();
                    }
                }
                else
                {
                    if ((!g_hotspotParams.empty()) && (spotType==0))
                    {
                        paramsInsertion(firstPos,hotSpot,checkPoint);
                    } else
                    {
                        normalSpotTriggered = true;
                    }

                    limitCounter++;
                }
            }
        }
        
    } while ((spotComplete>=0) && (spot>0) && (limitCounter<pc.configInt[CHAIN_LIMIT]) && !((g_hotspotParams.empty()) && (normalSpotTriggered))  );  // && (spotType!=0)

    //TODO: loosen the limit to the limit of special spot, and ++limit for every search so that less frezze will happen
    if (limitCounter>=pc.configInt[CHAIN_LIMIT]) showMessageBox(TEXT("Dynamic hotspots triggering limit exceeded."));
    //if (limitCounter>=pc.configInt[CHAIN_LIMIT]) ::MessageBox(nppData._nppHandle, TEXT("Dynamic hotspots triggering limit exceeded."), TEXT(PLUGIN_NAME), MB_OK);
    
	if (limitCounter > 0)
	{
		if (hotSpot)
		{
			delete[] hotSpot;
		}
		if (hotSpotText)
		{
			delete[] hotSpotText;
		}
        return true;
    }
    return false;
}

void paramsInsertion(int &firstPos, char* hotSpot, int &checkPoint)
{
    if (!g_hotspotParams.empty())
    {
        //alertString(*g_hotspotParams.begin());
        char* hotspotParamsCharArray = new char [(*g_hotspotParams.begin()).size()+1];
        strcpy(hotspotParamsCharArray, (*g_hotspotParams.begin()).c_str());
        
        g_hotspotParams.erase(g_hotspotParams.begin());
        
        if (strlen(hotspotParamsCharArray)>0)
        {
            
            //::SendScintilla(SCI_SETSEL,firstPos,secondPos+3);
            bool first = true;
            int found;
            do
            {
                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                found = searchNext(hotSpot);
                //endPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                //::SendScintilla(SCI_SETSEL,endPos-strlen(hotSpot),endPos);
                if (found >=0)
                {
                    if (first)
                    {
                        checkPoint = checkPoint - strlen(hotSpot) +strlen(hotspotParamsCharArray);
                        first = false;
                    }
                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)hotspotParamsCharArray);
                }
                //alertNumber(found);
            } while (found >= 0);
        } else
        {
            
        }

        delete [] hotspotParamsCharArray;
    }
}


void chainSnippet(int &firstPos, char* hotSpotText)
{
    //TODO: there may be a bug here. When the chain snippet contains content with CUT, the firstPos is not updated.
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_GOTOPOS,triggerPos,0);
    triggerTag(triggerPos,strlen(hotSpotText));
}


void webRequest(int &firstPos, char* hotSpotText)
{
    TCHAR requestType[20];
    int requestTypeLength = 0;

    if (strncmp(hotSpotText,"GET:",4)==0)
    {
        _tcscpy(requestType,TEXT("GET"));
        requestTypeLength = 4;
    } else if (strncmp(hotSpotText,"POST:",5)==0)
    {
        _tcscpy(requestType,TEXT("POST"));
        requestTypeLength = 5;
    } else if (strncmp(hotSpotText,"OPTIONS:",8)==0)
    {
        _tcscpy(requestType,TEXT("OPTIONS"));
        requestTypeLength = 8;
    } else if (strncmp(hotSpotText,"PUT:",4)==0)
    {
        _tcscpy(requestType,TEXT("PUT"));
        requestTypeLength = 5;
    } else if (strncmp(hotSpotText,"HEAD:",5)==0)
    {
        _tcscpy(requestType,TEXT("HEAD"));
        requestTypeLength = 5;
    } else if (strncmp(hotSpotText,"DELETE:",7)==0)
    {
        _tcscpy(requestType,TEXT("DELETE"));
        requestTypeLength = 7;
    } else if (strncmp(hotSpotText,"TRACE:",6)==0)
    {
        _tcscpy(requestType,TEXT("TRACE"));
        requestTypeLength = 6;
    } else if (strncmp(hotSpotText,"CONNECT:",8)==0)
    {
        _tcscpy(requestType,TEXT("CONNECT"));
        requestTypeLength = 8;
    } else
    {
        _tcscpy(requestType,TEXT("GET"));
        requestTypeLength = 0;
    }

    if (requestTypeLength>0)
    {
        SendScintilla(SCI_SETSEL,firstPos,firstPos+requestTypeLength);
        SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
    }

        
    int triggerPos = strlen(hotSpotText)+firstPos-requestTypeLength;
    //TODO: rewrite this part so that it doesn't rely on searchNext, and separate it out to another function to prepare for the implementation of "web snippet import"
    SendScintilla(SCI_GOTOPOS,firstPos,0);
    int spot1 = searchNext("://");
    int serverStart; 
    if ((spot1<0) || (spot1>triggerPos))
    {
        serverStart = firstPos;
    } else
    {
        serverStart = (SendScintilla(SCI_GETCURRENTPOS,0,0))+3;
    }
    SendScintilla(SCI_GOTOPOS,serverStart,0);

    int spot2 = searchNext("/");

    int serverEnd;
    if (spot2<0 || spot1>triggerPos)
    {
        serverEnd = triggerPos;
    } else
    {
        serverEnd = SendScintilla(SCI_GETCURRENTPOS,0,0);
    }

    if (serverEnd - serverStart > 0)
    {

        //char* server = new char[serverEnd-serverStart+1];
        //::SendScintilla(SCI_SETSELECTION,serverStart,serverEnd);
        //::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(server));

        char* server;
        sciGetText(&server,serverStart,serverEnd);

        TCHAR* serverWide = toWideChar(server);

        TCHAR* requestWide = toWideChar(hotSpotText + serverEnd - firstPos);
        
        //TODO: customizing type of request
        httpToFile(serverWide,requestWide,requestType,g_currentFocusPath);
        
        delete [] serverWide;
        delete [] requestWide;
        delete [] server;
    }

    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
}


////Old implementation of executeCommand
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
    //TODO: cater the problem that the path can have spaces..... as shown in the security remarks in http://msdn.microsoft.com/en-us/library/ms682425%28v=vs.85%29.aspx

    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");

    bool silent = false;
    int offset = 0;
    if (strncmp(hotSpotText,"SILENT:",7)==0)
    {
        silent = true;
        offset = 7;
    }


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

    TCHAR* cmdLine = toWideChar(hotSpotText+offset);

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

    if (silent)
    {
        si.wShowWindow = SW_SHOW;

    }

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
        strcat(hotSpotTextCmd,hotSpotText+offset);
        //strcpy(hotSpotTextCmd, hotSpotText);
        TCHAR* cmdLine2 = toWideChar(hotSpotTextCmd);

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
        } else
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }

        delete [] cmdLine2;
        delete [] hotSpotTextCmd;
    

    } else
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    
    delete [] cmdLine;
    
    
    //TODO: investigate the possibility to delay reading this part. So the process will keep giving output but I paste it into the editor when I see fit.
    if (!silent)
    {
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
    }

   //::SendScintilla(SCI_INSERTTEXT, 0, (LPARAM)"->End of process execution.\n");
}


std::string evaluateCall(char* expression)
{
    //TODO: can use smartsplit for string comparison (need to spply firstPos and TriggerPos instead of hotSpotExt for this function)
    //TODO: refactor the numeric comparison part
    std::vector<std::string> expressions;
    
    expressions = toVectorString(expression,';');
    bool stringComparison = false;

    int i = 0;
    int j = 0;
    for (i = 0;i<expressions.size();i++)
    {
        
        if (expressions[i].length()>1)
        {
            if ((expressions[i][0] != '"') || (expressions[i][expressions[i].length()-1] != '"'))
            {
                // This is math expression
                //Expression y(expressions[i]);
                //if ((y.evaluate(expressions[i])) == 0)
                //{
                if (execDuckEval(expressions[i]) == 0)
                {
                } else 
                {
                    expressions[i] = "error";
                    stringComparison = true;
                }
            } 
            else
            {
                // This is string
                expressions[i] = expressions[i].substr(1,expressions[i].length()-2);
                stringComparison = true;
            }
        } else if (expressions[i].length()>0)
        {
            // This is math expression
            //Expression y(expressions[i]);
            //if ((y.evaluate(expressions[i])) == 0)
            //{
            
            if (execDuckEval(expressions[i]) == 0)
            {
            } else 
            {
                expressions[i] = "error";
                stringComparison = true;   
            }
        } else
        {
            // This is string
            expressions[i] = "";
            stringComparison = true;
        }
    
    }
    
    std::string evaluateResult = "";
    double compareResult = 0;
    double storedCompareResult = 0;
    
    if (expressions.size() == 1)
    {
        evaluateResult = expressions[0];
    } else
    {
        
        if (stringComparison)
        {
            for (j = 1;j<expressions.size();j++)
            {
                for (i = j;i<expressions.size();i++)
                {
                    compareResult = expressions[i].compare(expressions[j-1]);
                    if (::abs(compareResult) > ::abs(storedCompareResult)) storedCompareResult = compareResult;
                }
            }
        
        } else
        {
            for (j = 1;j<expressions.size();j++)
            {
                for (i = j;i<expressions.size();i++)
                {
                    //std::stringstream ss0(expressions[j-1]);
                    //std::stringstream ss1(expressions[i]);
                    //double d0;
                    //double d1;
                    //ss0 >> d0;
                    //ss1 >> d1;
                    double d0 = toDouble(expressions[j-1]);
                    double d1 = toDouble(expressions[i]);

                    compareResult = d0 - d1;
                    if (::abs(compareResult) > ::abs(storedCompareResult)) storedCompareResult = compareResult;
                }
            }
        }
        
        //std::stringstream ss;
        //ss << ::abs(storedCompareResult);
        //evaluateResult = ss.str();
        evaluateResult = toString((double)::abs(storedCompareResult));
    }
    return evaluateResult;

}

void evaluateHotSpot(int &firstPos, char* hotSpotText)
{
    std::string evaluateResult;
    //TODO: should allow for a more elaborate comparison output
    
    int triggerPos = strlen(hotSpotText)+firstPos;
    SendScintilla(SCI_GOTOPOS,firstPos,0);
    
    int mode = 0;
    char* preParam;
    char delimiter1 = '?';
    char delimiter2 = ':';
    std::string verboseText = " => ";
    int offset=0;
    
    if (strncmp(hotSpotText,"VERBOSE'",8) == 0)
    {
        mode = 1;
        
        ::SendScintilla(SCI_GOTOPOS,firstPos + 8,0);
        int delimitEnd = searchNext("':");
        
        if ((delimitEnd >= 0) && (delimitEnd < firstPos+strlen(hotSpotText)))
        {
        
            ::SendScintilla(SCI_SETSELECTION,firstPos + 8,delimitEnd);
            //optionDelimiter = new char[delimitEnd - (firstPos + 5 + 8) + 1];
            //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionDelimiter));
            sciGetText(&preParam, firstPos + 8, delimitEnd);
            verboseText = toString(preParam);
            delete [] preParam;
            ::SendScintilla(SCI_SETSELECTION,firstPos ,delimitEnd + 2);
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            offset = delimitEnd + 2 - firstPos;
            //secondPos = secondPos - (delimitEnd + 2 - (firstPos + 5));
            
        } 
    } if (strncmp(hotSpotText,"TERNARY'",8) == 0)
    {
        mode = 0;
        
        ::SendScintilla(SCI_GOTOPOS,firstPos + 8,0);
        int delimitEnd = searchNext("':");
        
        if ((delimitEnd >= 0) && (delimitEnd < firstPos+strlen(hotSpotText)))
        {
        
            ::SendScintilla(SCI_SETSELECTION,firstPos + 8,delimitEnd);
            //optionDelimiter = new char[delimitEnd - (firstPos + 5 + 8) + 1];
            //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionDelimiter));
            sciGetText(&preParam, firstPos + 8, delimitEnd);
            delimiter1 = preParam[0];
            if (strlen(preParam)>=2)
            {
                delimiter2 = preParam[1];
            } else
            {
                delimiter2 = preParam[0];
            }
            
            delete [] preParam;
            ::SendScintilla(SCI_SETSELECTION,firstPos ,delimitEnd + 2);
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            offset = delimitEnd + 2 - firstPos;
            //secondPos = secondPos - (delimitEnd + 2 - (firstPos + 5));
            
        } 
    } else if (strncmp(hotSpotText,"TERNARY:",8) == 0)
    {
        ::SendScintilla(SCI_SETSELECTION,firstPos , firstPos+8);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        offset = 8;
        mode = 0;
    } else if (strncmp(hotSpotText,"VERBOSE:",8) == 0)
    {
        ::SendScintilla(SCI_SETSELECTION,firstPos, firstPos+8);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        offset = 8;
        mode = 1;
    }

    std::vector<std::string> firstSplit;

    firstSplit = smartSplit(firstPos,triggerPos-offset,delimiter1,2);

    if (firstSplit.size()<=1)
    {
        // Simple statament
        evaluateResult = evaluateCall(hotSpotText+offset);

    } else
    {
        // possible ternary statement
        std::vector<std::string> secondSplit;
        secondSplit = smartSplit(firstPos + firstSplit[0].length() + 1 ,triggerPos,delimiter2);  
        
        char* expression = toCharArray(firstSplit[0]);
        evaluateResult = evaluateCall(expression);

        //std::stringstream ss(evaluateResult);
        //double d;
        //ss>>d;

        double d = toDouble(evaluateResult);
        
        if (d > secondSplit.size()-1) d = secondSplit.size()-1;
        else if (d < 0) d = 0;
               
        evaluateResult = secondSplit[d];

    }

    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos-offset);
    if (mode == 1)
    {
        std::stringstream ss;
        ss << firstSplit[0] << verboseText << evaluateResult;
        evaluateResult = ss.str();
    } 

    char* result = toCharArray(evaluateResult);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)result);
    delete [] result;
}

void launchMessageBox(int &firstPos, char* hotSpotText)
{
    //TODO: need to find a better way to organize different types of messageboxes, there is probably no need to include all of them
    int triggerPos = strlen(hotSpotText)+firstPos;
    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);

    char* getTerm;
    getTerm = new char[strlen(hotSpotText)];
    strcpy(getTerm,"");
    
    // TODO: probably can just translate the text like "MB_OK" to the corresponding number and send it to the messagebox message directly. In this case people can just follow microsoft documentation.   
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
        delete [] getTerm;    //TODO: probably should have some default behaviour here?
        //TODO: Confirm to use return here, it stops the msg box launching when the msgbox type is unknown
        return;
    }

    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
    TCHAR* getTermWide = toWideChar(getTerm);
    int retVal = 0;
    retVal = showMessageBox(getTermWide,messageType);
    //retVal = ::MessageBox(nppData._nppHandle, getTermWide, TEXT(PLUGIN_NAME), messageType);
    char countText[10];
    ::_itoa(retVal, countText, 10);
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)countText);
    delete [] getTerm;
    delete [] getTermWide;
}

void textCopyCut(int sourceType, int operationType, int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint)
{
    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
    if (firstPos != 0)
    {
        int scriptStart;
        int selectionStart;
        int selectionEnd;

        //::SendScintilla(SCI_SETSEL,firstPos-1,firstPos);
        //::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::SendScintilla(SCI_GOTOPOS,firstPos-1,0);

        if (sourceType == 1)
        {
            ::SendScintilla(SCI_WORDLEFTEXTEND,0,0);

        } else if (sourceType == 2)     
        {
            ::SendScintilla(SCI_HOMEEXTEND,0,0);

        } else if (sourceType == 3)
        {
            ::SendScintilla(SCI_DOCUMENTSTARTEXTEND,0,0);

        } else if (sourceType == 4) 
        {
            int keywordLength;
            if (operationType == 0) keywordLength = 8;
            else if (operationType == 1) keywordLength = 9;
            else if (operationType == 2) keywordLength = 10;
            else if (operationType == 3) keywordLength = 11;

            int paramNumber = 0;

            char* getTerm;
            getTerm = new char[strlen(hotSpotText)];
            strcpy(getTerm,hotSpotText+keywordLength);
            
            paramNumber = ::atoi(getTerm);

            delete [] getTerm;

            if (paramNumber > 0) 
            {
            } else 
            {
                paramNumber = 1;
            }
            
            int targetLine = (::SendScintilla(SCI_LINEFROMPOSITION,firstPos-1,0)) - paramNumber + 1;
            if (targetLine<0) targetLine = 0;
            int targetPos = ::SendScintilla(SCI_POSITIONFROMLINE,targetLine,0);
            ::SendScintilla(SCI_SETSELECTION, targetPos, firstPos-1);


        } else if (sourceType == 5)
        {
            int keywordLength;
            if (operationType == 0) keywordLength = 4;
            else if (operationType == 1) keywordLength = 5;
            else if (operationType == 2) keywordLength = 6;
            else if (operationType == 3) keywordLength = 7;

            char* getTerm;
            getTerm = new char[strlen(hotSpotText)];
            strcpy(getTerm,hotSpotText+keywordLength);

            int scriptFound = -1;
            if (strlen(getTerm)>0) scriptFound = searchPrev(getTerm);

            delete [] getTerm;
            
            selectionEnd = firstPos-1; // -1 because the space before the snippet tag should not be included

            if (scriptFound>=0)
            {
                scriptStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                selectionStart = scriptStart + strlen(hotSpotText) - keywordLength;
                if (selectionEnd < selectionStart) selectionStart = selectionEnd;
                ::SendScintilla(SCI_SETSEL,selectionStart,selectionEnd);
            } else
            {

                ::SendScintilla(SCI_WORDLEFTEXTEND,0,0);
                scriptStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                selectionStart = scriptStart;
            }

            
        }

        if (operationType == 0)
        {
            startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            if (sourceType == 5) startingPos = scriptStart;
            if (checkPoint > startingPos) checkPoint = startingPos;  
            if (g_lastTriggerPosition > startingPos) g_lastTriggerPosition = startingPos;
            ::SendScintilla(SCI_CUT,0,0);
            if (sourceType == 5)
            {
                ::SendScintilla(SCI_SETSEL,scriptStart,selectionStart); //Delete the search key
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            }
        } else if (operationType == 1)  
        {
            ::SendScintilla(SCI_COPY,0,0);
            
        } else if (operationType == 2)
        {
            startingPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            if (sourceType == 5) startingPos = scriptStart;
            if (checkPoint > startingPos) checkPoint = startingPos;
            if (g_lastTriggerPosition > startingPos) g_lastTriggerPosition = startingPos;
            //delete [] g_customClipBoard;
            //g_customClipBoard = new char [(::SendScintilla(SCI_GETSELECTIONEND,0,0)) - (::SendScintilla(SCI_GETSELECTIONSTART,0,0)) +1];
            //::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(g_customClipBoard));
            char* tempCustomClipBoardText;
            sciGetText(&tempCustomClipBoardText,(::SendScintilla(SCI_GETSELECTIONSTART,0,0)),(::SendScintilla(SCI_GETSELECTIONEND,0,0)));
            g_customClipBoard = toString(tempCustomClipBoardText);
            delete [] tempCustomClipBoardText;
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            if (sourceType == 5)
            {
                ::SendScintilla(SCI_SETSEL,scriptStart,selectionStart); //Delete the search key
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            }

        } else if (operationType == 3)
        {
            char* tempCustomClipBoardText;
            sciGetText(&tempCustomClipBoardText,(::SendScintilla(SCI_GETSELECTIONSTART,0,0)),(::SendScintilla(SCI_GETSELECTIONEND,0,0)));
            g_customClipBoard = toString(tempCustomClipBoardText);
            delete [] tempCustomClipBoardText;
        }
    }

}


void keyWordSpot(int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint)
{
    int hotSpotTextLength = strlen(hotSpotText);
    int triggerPos = hotSpotTextLength+firstPos;

    ::SendScintilla(SCI_SETSEL,firstPos,triggerPos);
    //TODO: At least I should rearrange the keyword a little bit for efficiency
    //TODO: refactor the logic of checking colon version, for example DATE and DATE: for efficiency
    //TODO: remove the GET series
    //TODO: all keywords should have a "no colon" version and show error message of "params needed"

    if (strcmp(hotSpotText,"PASTE") == 0)
    {
        ::SendScintilla(SCI_PASTE,0,0);
	    
    } else if (strcmp(hotSpotText,"FTPASTE") == 0)
    {
        char* tempCustomClipBoardText = toCharArray(g_customClipBoard);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tempCustomClipBoardText);
        delete [] tempCustomClipBoardText;
    } else if ((strcmp(hotSpotText,"CUT") == 0) || (strcmp(hotSpotText,"CUTWORD") == 0))
    {
        textCopyCut(1, 0, firstPos, hotSpotText, startingPos, checkPoint); 
    } else if ((strcmp(hotSpotText,"COPY") == 0) || (strcmp(hotSpotText,"COPYWORD") == 0) || (strcmp(hotSpotText,"GET") == 0))
    {
        textCopyCut(1, 1, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"CUTLINE") == 0)
    {
        textCopyCut(2, 0, firstPos, hotSpotText, startingPos, checkPoint);
    } else if ((strcmp(hotSpotText,"COPYLINE") == 0) || (strcmp(hotSpotText,"GETLINE") == 0))
    {
        textCopyCut(2, 1, firstPos, hotSpotText, startingPos, checkPoint);
    } else if ((strcmp(hotSpotText,"CUTDOC") == 0) || (strcmp(hotSpotText,"CUTALL") == 0))
    {
        textCopyCut(3, 0, firstPos, hotSpotText, startingPos, checkPoint);
    } else if ((strcmp(hotSpotText,"COPYDOC") == 0) || (strcmp(hotSpotText,"GETALL") == 0))
    {
        textCopyCut(3, 1, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"CUTLINE:",8) == 0)
    {
        textCopyCut(4, 0, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"COPYLINE:",9) == 0)
    {
        textCopyCut(4, 1, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"CUT:",4) == 0) 
    {
        textCopyCut(5, 0, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"COPY:",5) == 0) 
    {
        textCopyCut(5, 1, firstPos, hotSpotText, startingPos, checkPoint);
    } else if ((strcmp(hotSpotText,"FTCUT") == 0) || (strcmp(hotSpotText,"FTCUTWORD") == 0))
    {
        textCopyCut(1, 2, firstPos, hotSpotText, startingPos, checkPoint); 
    } else if ((strcmp(hotSpotText,"FTCOPY") == 0) || (strcmp(hotSpotText,"FTCOPYWORD") == 0))
    {
        textCopyCut(1, 3, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"FTCUTLINE") == 0)
    {
        textCopyCut(2, 2, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"FTCOPYLINE") == 0)
    {
        textCopyCut(2, 3, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"FTCUTDOC") == 0)
    {
        textCopyCut(3, 2, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"FTCOPYDOC") == 0)
    {
        textCopyCut(3, 3, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"FTCUTLINE:",10) == 0)
    {
        textCopyCut(4, 2, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"FTCOPYLINE:",11) == 0)
    {
        textCopyCut(4, 3, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"FTCUT:",6) == 0) 
    {
        textCopyCut(5, 2, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strncmp(hotSpotText,"FTCOPY:",7) == 0) 
    {
        textCopyCut(5, 3, firstPos, hotSpotText, startingPos, checkPoint);
    } else if (strcmp(hotSpotText,"SELECTION") == 0)
    {
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)g_selectedText.c_str());
    } else if (strcmp(hotSpotText,"SELECTIONORPASTE") == 0)
    {
        if (g_selectedText.length() <=0)
        {
            ::SendScintilla(SCI_PASTE,0,0);
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)g_selectedText.c_str());
        }
    } else if (strcmp(hotSpotText,"TEMPFILE") == 0)
    {
        insertPath(g_fttempPath);

    } else if (strcmp(hotSpotText,"FILEFOCUS") == 0)
    {
        insertPath(g_currentFocusPath);

    } else if (strncmp(hotSpotText,"TEMPFILE:",9)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+9);
        std::string delimiter = toString(getTerm);
        insertPath(g_fttempPath,delimiter);
        delete [] getTerm;

    } else if (strncmp(hotSpotText,"FILEFOCUS:",10)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+10);
        std::string delimiter = toString(getTerm);
        insertPath(g_currentFocusPath,delimiter);
        delete [] getTerm;

    } else if (strncmp(hotSpotText,"SETFILE:",8) == 0)  //TODO: a lot of refactoring needed for the file series
    {
        char* getTerm;
        
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+8);
        TCHAR* getTermWide = toWideChar(getTerm);
        if (strlen(getTerm) < MAX_PATH)
        {
            ::_tcscpy_s(g_currentFocusPath,getTermWide);
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            emptyFile(g_currentFocusPath);
        } else 
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file name is too long.");
        }

        delete [] getTerm;
        delete [] getTermWide;
        
    } else if (strncmp(hotSpotText,"WRITE:",6) == 0)  
    {
        //TODO: Should have Append mode  
        //TODO: refactor file wriiting to another function
        emptyFile(g_currentFocusPath);
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        std::ofstream fileStream(g_currentFocusPath, std::ios::binary); // need to open in binary so that there will not be extra spaces written to the document
        if (fileStream.is_open())
        {
            fileStream << getTerm;
            fileStream.close();
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"WRITETEMP:",10) == 0)
    {
        emptyFile(g_fttempPath);
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+10);
        std::ofstream fileStream(g_fttempPath, std::ios::binary); // need to open in binary so that there will not be extra spaces written to the document
        if (fileStream.is_open())
        {
            fileStream << getTerm;
            fileStream.close();
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }
        delete [] getTerm;
    } else if (strcmp(hotSpotText,"FTWRITE") == 0)
    {
        emptyFile(g_currentFocusPath);
        
        std::ofstream fileStream(g_currentFocusPath, std::ios::binary); // need to open in binary so that there will not be extra spaces written to the document
        if (fileStream.is_open())
        {
            fileStream << g_customClipBoard;
            fileStream.close();
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }

    } else if (strcmp(hotSpotText,"FTWRITETEMP") == 0)
    {
        emptyFile(g_fttempPath);
        
        std::ofstream fileStream(g_fttempPath, std::ios::binary); // need to open in binary so that there will not be extra spaces written to the document
        if (fileStream.is_open())
        {
            fileStream << g_customClipBoard;
            fileStream.close();
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }

    } else if (strcmp(hotSpotText,"READ") == 0)
    {
        char* buffer;

        std::ifstream fileStream;
        fileStream.open(g_currentFocusPath, std::ios::binary | std::ios::in);

        if (fileStream.is_open())
        {
            fileStream.seekg (0, std::ios::end);
            int length = fileStream.tellg();
            fileStream.seekg (0, std::ios::beg);

            buffer = new char [length+1];
            
            fileStream.read (buffer,length);
            buffer[length] = '\0';
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)buffer);
            fileStream.close();
            delete[] buffer;
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }
  
    } else if (strcmp(hotSpotText,"READTEMP") == 0)
    {
        char* buffer;
        
        std::ifstream fileStream;
        fileStream.open(g_fttempPath, std::ios::binary | std::ios::in);
        if (fileStream.is_open())
        {
            fileStream.seekg (0, std::ios::end);
            int length = fileStream.tellg();
            fileStream.seekg (0, std::ios::beg);

            buffer = new char [length+1];
            
            fileStream.read (buffer,length);
            buffer[length] = '\0';
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)buffer);
            fileStream.close();
            delete[] buffer;
        } else
        {
            ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"The file cannot be opened.");
        }
    } else if (strncmp(hotSpotText,"UPPER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)::_strupr(getTerm));
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"LOWER:",6)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)::_strlwr(getTerm));
        delete [] getTerm;
    } else if (strcmp(hotSpotText,"WINFOCUS")==0)
    {
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        setFocusToWindow();
    } else if (strcmp(hotSpotText,"SETWIN")==0)
    {
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::searchWindowByName("");
        
    } else if (strncmp(hotSpotText,"SETWIN:",7)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+7);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::std::string tempString(getTerm);
        ::searchWindowByName(tempString);
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"SETCHILD:",9)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+9);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        ::std::string tempString(getTerm);
        ::searchWindowByName(tempString,g_tempWindowHandle);
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"KEYDOWN:",8)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+8);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        setFocusToWindow();
        generateKey(toVk(getTerm),true);
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"KEYUP:",6)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        setFocusToWindow();
        generateKey(toVk(getTerm),false);
        delete [] getTerm;
    }  else if (strncmp(hotSpotText,"KEYHIT:",7)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+7);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        setFocusToWindow();
        generateKey(toVk(getTerm),true);
        generateKey(toVk(getTerm),false);
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"SCOPE:",6)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        //TCHAR* scopeWide = new TCHAR[strlen(getTerm)*4+!];
        pc.configText[CUSTOM_SCOPE] = toWideChar(getTerm);  //TODO: memory leak here?
        pc.callWriteConfigText(CUSTOM_SCOPE);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        updateDockItems(true,false,"%",true);
        //snippetHintUpdate();
        delete [] getTerm;
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
    } else if (strncmp(hotSpotText,"DATE:",5)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+5);
        char* dateReturn = getDateTime(getTerm);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)dateReturn);
        delete [] dateReturn;
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"TIME:",5)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+5);
        char* timeReturn = getDateTime(getTerm,false);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)timeReturn);
        delete [] timeReturn;
        delete [] getTerm;
    } else if (strncmp(hotSpotText,"REPLACE:",8)==0)
    {
        //TODO: cater backward search (need to investigate how to set the checkpoint, firstpos and starting pos
        //TODO: allow custom separator
        std::vector<std::string> params = smartSplit(firstPos + 8, triggerPos,',');
        SendScintilla(SCI_SETSEL,firstPos,triggerPos);
        SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        if (params.size() > 1)
        {
            searchAndReplace(params[0],params[1]);
        } else
        {
            //TODO: error message of not enough params
        }
        
    }  else if (strncmp(hotSpotText,"REGEXREPLACE:",13)==0)
    {
        std::vector<std::string> params = smartSplit(firstPos + 13, triggerPos,',');
        SendScintilla(SCI_SETSEL,firstPos,triggerPos);
        SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        if (params.size() > 1)
        {
            searchAndReplace(params[0],params[1],true);
        } else
        {
            //TODO: error message of not enough params
        }
        
    } else if (strcmp(hotSpotText,"FILENAME")==0)
    {
        insertNppPath(NPPM_GETNAMEPART);
        
    } else if (strcmp(hotSpotText,"EXTNAME")==0)
    {
        insertNppPath(NPPM_GETEXTPART);
        
    } else if (strcmp(hotSpotText,"DIRECTORY")==0)
    {
        insertNppPath(NPPM_GETCURRENTDIRECTORY);
    } else if (strncmp(hotSpotText,"DIRECTORY:",10)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+10);
        std::string delimiter = toString(getTerm);
        insertNppPath(NPPM_GETCURRENTDIRECTORY,delimiter);
        delete [] getTerm;

    } else if (strncmp(hotSpotText,"SLEEP:",6)==0)
    {
        char* getTerm;
        getTerm = new char[hotSpotTextLength];
        strcpy(getTerm,hotSpotText+6);
        
        int sleepLength = ::atoi(getTerm);
        if (sleepLength>60000)
        {
            sleepLength = 60000;
        } else if (sleepLength<0)
        {
            sleepLength = 0;
        }
        ::Sleep(sleepLength);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
        delete [] getTerm;
        
    } else if ((strncmp(hotSpotText,"CLEAN_",6)==0) && (strncmp(hotSpotText+7,":",1)==0))
    {
        // TODO: fill in content for this CLEAN keyword
        
    } else if ((strncmp(hotSpotText,"COUNT_",6)==0) && (strncmp(hotSpotText+7,":",1)==0))
    {
        // TODO: fill in content for this COUNT keyword
    } else if (strcmp(hotSpotText,"TRIGGERTEXT") == 0) 
    {
        char* lastTriggerTextText = toCharArray(g_lastTriggerText);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)lastTriggerTextText);
        delete [] lastTriggerTextText;
    } else if (strcmp(hotSpotText,"LASTOPTION") == 0) 
    {
        char* lastOptionText = toCharArray(g_lastOption);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)lastOptionText);
        delete [] lastOptionText;
    } else if (strcmp(hotSpotText,"LASTLISTITEM") == 0) 
    {
        char* lastListItemText = toCharArray(g_lastListItem);
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)lastListItemText);
        delete [] lastListItemText;
    } else 
    {
        char* errorMessage = new char[hotSpotTextLength+40];
        if (hotSpotTextLength>0)
        {
            strcpy(errorMessage,"'");
            strcat(errorMessage, hotSpotText);
            strcat(errorMessage,"' is not a keyword in fingertext.");
        } else
        {
            strcpy(errorMessage,"Keyword missing.");
        }
        
        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)errorMessage);
        delete [] errorMessage;
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

void searchAndReplace(std::string key, std::string text, bool regexp)
{
    char* searchKey = new char[key.length()+1];
    char* replaceText = new char[text.length()+1];
    strcpy(searchKey,key.c_str());
    strcpy(replaceText,text.c_str());
    int keySpot = -1;
    keySpot = searchNext(searchKey,regexp);

    while (keySpot >= 0)
    {
        if (keySpot >= 0)
        {
            SendScintilla(SCI_REPLACESEL,0,(LPARAM)replaceText);
        }
        keySpot = searchNext(searchKey,regexp);
    }
}

void insertPath(TCHAR* path, std::string delimiter)
{
    char* pathText = toCharArray(path,(int)::SendScintilla(SCI_GETCODEPAGE, 0, 0));
    std::string pathTextString = toString(pathText);
    findAndReplace(pathTextString,"\\",delimiter);
    //findAndReplace(pathTextString,"\\","\\\\");
    char* pathTextReplaced = toCharArray(pathTextString);
    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)pathTextReplaced);
    delete [] pathTextReplaced;
    delete [] pathText;
}

void insertNppPath(int msg, std::string delimiter)
{
	TCHAR path[MAX_PATH];
	::SendMessage(nppData._nppHandle, msg, 0, (LPARAM)path);

    insertPath(path,delimiter);

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
    if (format)
    {
        formatWide = toWideChar(format);
    } else
    {
        formatWide = NULL;
    }
    
    if (getDate)
    {
        ::GetDateFormat(LOCALE_USER_DEFAULT, flags, &formatTime, formatWide, result, 128);
    } else
    {
        ::GetTimeFormat(LOCALE_USER_DEFAULT, flags, &formatTime, formatWide, result, 128);
    }
    
    if (format) delete [] formatWide;
    
    char* resultText = toCharArray(result,(int)::SendScintilla(SCI_GETCODEPAGE, 0, 0));
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

bool fingerTextListActive()
{
    if (g_fingerTextList)
    {
        g_fingerTextList = false;
        return true;
    } else
    {
        return false;
    }
}


int hotSpotNavigation(char* tagSign, char* tagTail)
{
    int retVal = 0;
    // TODO: consolidate this part with dynamic hotspots? 

    //char tagSign[] = "$[![";
    //int tagSignLength = strlen(tagSign);
    //char tagTail[] = "]!]";
    //int tagTailLength = strlen(tagTail);
    int tagSignLength = 4;

    char *hotSpotText;
    char *hotSpot;

    int tagSpot = searchNext(tagTail);    // Find the tail first so that nested snippets are triggered correctly
    
	if (tagSpot >= 0)
	{
        
        if (pc.configInt[PRESERVE_STEPS] == 0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);

        //int tailPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
        if (searchPrev(tagSign) >= 0)
        {
            int firstPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            int secondPos = 0;
            grabHotSpotContent(&hotSpotText, &hotSpot, firstPos, secondPos, tagSignLength, tagSpot);

            if (strncmp(hotSpotText,"(lis)",5) == 0)
            {
                g_fingerTextList = true;
                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"");

                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                
                ::SendScintilla(SCI_AUTOCSETSEPARATOR, (LPARAM)'|', 0); 
                ::SendScintilla(SCI_AUTOCSHOW, 0, (LPARAM)(hotSpotText+5));
                retVal = 3;

            } else if (strncmp(hotSpotText,"(opt)",5) == 0)
            {
                
                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)hotSpotText);
                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                int triggerPos = firstPos + strlen(hotSpotText);
                ::SendScintilla(SCI_SETSELECTION,firstPos,firstPos+5);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                triggerPos -= 5;

                //TODO: refactor the option hotspot part to a function
                int mode = 0;
                char* preParam;
                char delimiter = '|';
                int offset=0;
                
                if (strncmp(hotSpotText+5,"DELIMIT'",8) == 0)   // TODO: the +5 is not necessary, should delete the (opt) first......
                {
                    mode = 0;
                    ::SendScintilla(SCI_GOTOPOS,firstPos + 8,0);
                    int delimitEnd = searchNext("':");
                    
                    if ((delimitEnd >= 0) && (delimitEnd < triggerPos))
                    {

                        ::SendScintilla(SCI_SETSELECTION,firstPos + 8,delimitEnd);
                        //optionDelimiter = new char[delimitEnd - (firstPos + 5 + 8) + 1];
                        //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionDelimiter));
                        sciGetText(&preParam, firstPos + 8, delimitEnd);
                        delimiter = preParam[0];
                        
                        delete [] preParam;
                        ::SendScintilla(SCI_SETSELECTION,firstPos,delimitEnd + 2);
                        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                        offset = delimitEnd + 2 - firstPos;
                        //secondPos = secondPos - (delimitEnd + 2 - (firstPos + 5));
                    } 
                } else if (strncmp(hotSpotText+5,"DELIMIT:",8) == 0)
                {
                    mode = 0;
                    ::SendScintilla(SCI_SETSELECTION,firstPos, firstPos + 8);  
                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                    offset = 8;
                } else if (strncmp(hotSpotText+5,"RANGE:",6) == 0)
                {
                    //TODO: may allow for customizable delimiter for RANGE. but it shouldnt be necessary
                    mode = 1;
                    ::SendScintilla(SCI_SETSELECTION,firstPos, firstPos + 6);  
                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                    offset = 6;
                } 
                
                cleanOptionItem();

                if (mode == 1)
                {
                    char* getTerm;
                    sciGetText(&getTerm, firstPos,triggerPos-offset);
                    getTerm = quickStrip(getTerm,' ');
                    getTerm = quickStrip(getTerm,'\r');
                    getTerm = quickStrip(getTerm,'\n');
                    getTerm = quickStrip(getTerm,'\t');
                    std::vector<std::string> rangeString = toVectorString(getTerm,'-');
                    delete [] getTerm;

                    int numLength = 1;
                    long rangeStart;
                    long rangeEnd;
                    
                    if (rangeString.size()>1)
                    {   
                        if (rangeString[0].length()<=0) rangeString[0] = "0";
                        if (rangeString[1].length()<=0) rangeString[1] = "0";
                        
                        numLength = rangeString[0].length();
                        if (rangeString[1].length()<rangeString[0].length()) numLength = rangeString[1].length();
                        
                        rangeStart = toLong(rangeString[0]);
                        rangeEnd = toLong(rangeString[1]);
                        //std::stringstream ss1(rangeString[0]);
                        //ss1 >> rangeStart;
                        //
                        //std::stringstream ss2(rangeString[1]);
                        //ss2 >> rangeEnd;
                        //
                        rangeStart = abs(rangeStart);
                        rangeEnd = abs(rangeEnd);

                        int length;
                        if (rangeEnd>=rangeStart)
                        {
                            for (int i = rangeStart; i<=rangeEnd; i++)
                            {
                                std::string s = toString(i);
                                //std::stringstream ss;
                                //ss << i;
                                //std::string s = ss.str();
                                
                                length = s.length();
	                            for (int j = 0; j < numLength - length; j++) s = "0" + s;
                                g_optionArray.push_back(s);
                            }
                        } else
                        {
                            for (int i = rangeStart; i>=rangeEnd; i--)
                            {
                                std::string s = toString(i);
                                //std::stringstream ss;
                                //ss << i;
                                //std::string s = ss.str();

                                length = s.length();
	                            for (int j = 0; j < numLength - length; j++) s = "0" + s;
                                g_optionArray.push_back(s);
                            
                            }
                        }
                    } else
                    {
                        g_optionArray.push_back(rangeString[0]);
                    }
                } else
                {
                    g_optionArray = smartSplit(firstPos,triggerPos-offset,delimiter);
                }
                //g_optionNumber = g_optionNumber + g_optionArray.size();
                
                //tempOptionEnd = firstPos + 5 - strlen(optionDelimiter);
                //int i =0;
                //int optionFound = -1;
                ////while (i<g_optionArrayLength)
                //while(1)
                //{
                //    tempOptionStart = tempOptionEnd + strlen(optionDelimiter);
                //    ::SendScintilla(SCI_GOTOPOS,tempOptionStart,0);
                //    optionFound = searchNext(optionDelimiter);
                //    if ((optionFound>=0) && (::SendScintilla(SCI_GETCURRENTPOS,0,0)<secondPos))
                //    {
                //        tempOptionEnd = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                //        ::SendScintilla(SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                //        char* optionText;
                //        //optionText = new char[tempOptionEnd - tempOptionStart + 1];
                //        //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                //        sciGetText(&optionText, tempOptionStart, tempOptionEnd);
                //        addOptionItem(optionText);
                //        i++;
                //    } else
                //    {
                //        tempOptionEnd = secondPos-4;
                //        ::SendScintilla(SCI_SETSELECTION,tempOptionStart,tempOptionEnd);
                //        char* optionText;
                //        //optionText = new char[tempOptionEnd - tempOptionStart + 1];
                //        //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(optionText));
                //        sciGetText(&optionText, tempOptionStart, tempOptionEnd);
                //        addOptionItem(optionText);
                //        i++;
                //        
                //        break;
                //    }
                //};


                //g_optionOperating = true; 

                ::SendScintilla(SCI_SETSELECTION,firstPos,triggerPos-offset);
                char* option = toCharArray(g_optionArray[g_optionCurrent]);
                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)option);
                //g_optionOperating = false; 
                delete [] option;
                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                g_optionStartPosition = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                g_optionEndPosition = g_optionStartPosition + g_optionArray[g_optionCurrent].length();
                ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
                //alertNumber(g_optionArray.size());
                if (g_optionArray.size() > 1)
                {
                    turnOnOptionMode();
                    //g_optionMode = true;
                    updateOptionCurrent(true);
                }
                //alertNumber(::SendMessage(curScintilla,SCI_GETCURRENTPOS,0,0));
                
                //char* tempText;
                //tempText = new char[secondPos - firstPos + 1];
                //::SendMessage(curScintilla, SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(tempText));
                //alertCharArray(tempText);

                //::SendMessage(curScintilla,SCI_SETSELECTION,firstPos,secondPos);
                //::SendMessage(curScintilla, SCI_REPLACESEL, 0, (LPARAM)g_optionArray[g_optionCurrent]);
                retVal = 2;
            } else
            {
                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)hotSpotText);

                ::SendScintilla(SCI_GOTOPOS,firstPos,0);
                int hotSpotFound=-1;
                int tempPos[100];
                int i=1;
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
                //::SendScintilla(SCI_SCROLLCARET,0,0);
                
                
                ::SendScintilla(SCI_SETSELECTION,firstPos,secondPos-tagSignLength);
                for (int j=1;j<i;j++)
                {
                    if (tempPos[j]!=-1)
                    {
                        ::SendScintilla(SCI_ADDSELECTION,tempPos[j],tempPos[j]+(secondPos-tagSignLength-firstPos));
                    }
                }
                ::SendScintilla(SCI_SETMAINSELECTION,0,0);
                ::SendScintilla(SCI_SCROLLCARET,0,0);
                //::SendScintilla(SCI_LINESCROLL,0,20);
                //TODO: scrollcaret is not working correctly when thre is a dock visible
                retVal = 1;
            }

            delete [] hotSpot;
            delete [] hotSpotText;

        }
        if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
        
	} else
    {
        //delete [] hotSpot;  // Don't try to delete if it has not been initialized
        //delete [] hotSpotText;
        retVal = 0;
    }
    return retVal;
}

int grabHotSpotContent(char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength, int tailPos)
{
    int spotType = 0;

    //searchNext(tagTail);
    
	//secondPos = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
    secondPos = tailPos;

    //::SendScintilla(SCI_SETSELECTION,firstPos+signLength,secondPos);
    //*hotSpotText = new char[secondPos - (firstPos + signLength) + 1];
    //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpotText));
    sciGetText(&*hotSpotText, (firstPos + signLength), secondPos);

    if (strncmp(*hotSpotText,"(cha)",5)==0)
    {
        spotType = 1;
    } else if (strncmp(*hotSpotText,"(key)",5)==0)
    {
        spotType = 2;
    } else if ((strncmp(*hotSpotText,"(run)",5)==0) || (strncmp(*hotSpotText,"(cmd)",5)==0))  //TODO: the command hotspot is renamed to (run) this line is for keeping backward compatibility
    {
        spotType = 3;
    } else if (strncmp(*hotSpotText,"(msg)",5)==0) //TODO: should think more about this hotspot, It can be made into a more general (ask) hotspot....so keep this feature private for the moment
    {
        spotType = 4;
    } else if (strncmp(*hotSpotText,"(eva)",5)==0)
    {
        spotType = 5;
    } else if ((strncmp(*hotSpotText,"(web)",5)==0) || (strncmp(*hotSpotText,"(www)",5)==0))
    {
        spotType = 6;
    } else if (strncmp(*hotSpotText,"(opt)",5)==0)
    {
        spotType = -1;
    } else if (strncmp(*hotSpotText,"(lis)",5)==0)
    {
        spotType = -2;
    }  

    //::SendScintilla(SCI_SETSELECTION,firstPos,secondPos+3);
    //*hotSpot = new char[secondPos+3 - firstPos + 1];
    //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(*hotSpot));

    ::SendScintilla(SCI_SETSELECTION,firstPos,secondPos+3);
    sciGetText(&*hotSpot,firstPos,secondPos+3);
    
    return spotType;
    //return secondPos;  
}



void showPreview(bool top,bool insertion)
{
    
    TCHAR* bufferWide;

    int lineLengthLimit;
    int lineNumberLimit;
    if (insertion)
    {
        lineLengthLimit = 50;
        lineNumberLimit = 18;
    } else
    {

        lineLengthLimit = 40;
        lineNumberLimit = 7;
    }



    if (insertion)
    {
        if (top)
        {
            insertionDlg.getSelectText(bufferWide,0);
        } else
        {
            insertionDlg.getSelectText(bufferWide);   
        }
    } else
    {
        if (top)
        {
            snippetDock.getSelectText(bufferWide,0);
        } else
        {
            snippetDock.getSelectText(bufferWide);
        }
    }
    
    if (::_tcslen(bufferWide)>0)
    {

        char* buffer = toCharArray(bufferWide);
        buffer = quickStrip(buffer, ' ');

        int scopeLength = ::strchr(buffer,'>') - buffer - 1;
        int triggerTextLength = strlen(buffer)-scopeLength - 2;
        char* tempTriggerText = new char [ triggerTextLength+1];
        char* tempScope = new char[scopeLength+1];
        
        strncpy(tempScope,buffer+1,scopeLength);
        tempScope[scopeLength] = '\0';
        strncpy(tempTriggerText,buffer+1+scopeLength+1,triggerTextLength);
        tempTriggerText[triggerTextLength] = '\0';

        if (!top)
        {
            TCHAR* tempTriggerTextWide = toWideChar(tempTriggerText);
            insertionDlg.setDlgText(IDC_INSERTION_EDIT,tempTriggerTextWide);
            delete [] tempTriggerTextWide;
        }
        delete [] buffer;

        sqlite3_stmt *stmt;
        
        if (SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
	    {
	    	// Then bind the two ? parameters in the SQLite SQL to the real parameter values
	    	sqlite3_bind_text(stmt, 1, tempScope , -1, SQLITE_STATIC);
	    	sqlite3_bind_text(stmt, 2, tempTriggerText, -1, SQLITE_STATIC);

	    	// Run the query with sqlite3_step
	    	if(SQLITE_ROW == sqlite3_step(stmt))  // SQLITE_ROW 100 sqlite3_step() has another row ready
	    	{
                
                const char* snippetText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)); // The 0 here means we only take the first column returned. And it is the snippet as there is only one column
                
                int snippetEndPosition = (int)(toString((char*)snippetText).find("[>END<]"));

                char* tempSnippetComment = new char[strlen(snippetText) - snippetEndPosition + 1];
                strcpy(tempSnippetComment,snippetText+snippetEndPosition+7);
                TCHAR* tempSnippetCommentWide = toWideChar(tempSnippetComment);
                
                if (strlen(snippetText) - snippetEndPosition-7-1>0)
                {
                    insertionDlg.changeMode(true);

                } else
                {
                    insertionDlg.changeMode(false);
                }

                
                char* tempSnippetText = new char[snippetEndPosition+1];
                strncpy(tempSnippetText,snippetText,snippetEndPosition);
                tempSnippetText[snippetEndPosition] = '\0';

                tempSnippetText = quickStrip(tempSnippetText,'\r');
                std::vector<std::string> vs = toVectorString(tempSnippetText,'\n');


                char* previewText = new char[lineLengthLimit*(lineNumberLimit+1)];
                strcpy(previewText,"");
                
                int i = 0;
                do
                {
                    char* line = new char[lineLengthLimit];
                    if (vs[i].length()>lineLengthLimit-5) 
                    {
                        strncat(previewText,vs[i].c_str(),lineLengthLimit-9);
                        strcat(previewText, "......");
                    } else
                    {
                        strcat(previewText,vs[i].c_str());
                    }
                    //strcat(previewText,vs[i].c_str());
                    strcat(previewText,"\r\n");
                    delete [] line;
                    i++;
                    
                } while (i<vs.size() && i<lineNumberLimit-1);
                
                if (vs.size()>lineNumberLimit) strcat(previewText,"......");
                else if (vs.size()==lineNumberLimit)
                {
                    strcat(previewText,vs[lineNumberLimit-1].c_str());
                }
                
                TCHAR* previewTextWide = toWideChar(previewText);
                
                if (insertion)
                {
                    insertionDlg.setDlgText(IDC_INSERTION_PREVIEW,previewTextWide);
                    
                    insertionDlg.setDlgText(IDC_INSERTION_DES,tempSnippetCommentWide);
                    
                    int currentEditTextPos = insertionDlg.getEditPos();
                    
                    //::SendScintilla(SCI_REPLACESEL,0,(LPARAM)toCharArray(toString(currentEditTextPos)));
                    TCHAR* currentEditTextWide = insertionDlg.getEditText();
                    char* currentEditText = toCharArray(currentEditTextWide);
                    // TODO: cater the case where I enter something with spaces (should only read the last entry after breaking it down to vector)
                    
                    
                    std::vector<std::string> editTextVector = toVectorString(currentEditText,'(',2);
                    std::vector<std::string> editTextVector2;
                    if (editTextVector.size()>1)
                    {
                        editTextVector2 = toVectorString((char*)editTextVector[1].c_str(),',');
                    }
                    delete [] currentEditTextWide;
                    delete [] currentEditText;
                
                    if (editTextVector[0].length()>0)
                    {
                        //TODO: analyze the snippet content and extract the params insertion hint here
                    
                        std::vector<std::string> hintTextVector = snippetTextBrokenDown(editTextVector[0],editTextVector2,&tempTriggerText,&tempSnippetText,currentEditTextPos);
                    
                        TCHAR* hintText0 = toWideChar(hintTextVector[0]);
                        TCHAR* hintText1 = toWideChar(hintTextVector[1]);
                        TCHAR* hintText2 = toWideChar(hintTextVector[2]);
                    
                        insertionDlg.setDlgText(IDC_INSERTION_HINT,hintText0);
                        insertionDlg.setDlgText(IDC_INSERTION_HINT_HIGHLIGHT,hintText1);
                        insertionDlg.setDlgText(IDC_INSERTION_HINT_POST,hintText2);
                    
                        insertionDlg.adjustTextHintPosition();
                    
                        delete [] hintText0;
                        delete [] hintText1;
                        delete [] hintText2;
                    } else
                    {
                        insertionDlg.setDlgText(IDC_INSERTION_HINT,TEXT("Type the TriggerText of the snippet and press TAB to insert."));
                        insertionDlg.adjustTextHintPosition();
                    }
                    
                //
                } else
                {
                    
                    snippetDock.setDlgText(IDC_PREVIEW_EDIT,bufferWide);
                    snippetDock.setDlgText(ID_SNIPSHOW_EDIT,previewTextWide);
                }
                
                delete [] previewText;
                delete [] tempSnippetComment;
                delete [] tempSnippetCommentWide;
                delete [] tempSnippetText;
                delete [] previewTextWide;
	    	}	
	    }
	    sqlite3_finalize(stmt);

        delete [] tempTriggerText;
        delete [] tempScope;
    } else
    {
        if (insertion)
        {
            insertionDlg.setDlgText(IDC_INSERTION_PREVIEW,TEXT("No matching snippet."));
            insertionDlg.setDlgText(IDC_INSERTION_DES,TEXT(""));
        } else
        { 
            snippetDock.setDlgText(ID_SNIPSHOW_EDIT,TEXT("No matching snippet."));
        }
    }
    delete [] bufferWide;
    //::SendMessage(curScintilla,SCI_GRABFOCUS,0,0); 
}



std::vector<std::string> snippetTextBrokenDown(std::string editText, std::vector<std::string> params, char** tempTriggerText, char** snippetContent, int position)
{
    int location = 0;
    
    if (position<=editText.length())
    {
        location = 0;
    } else
    {
        location = 1;
        position = position - editText.length() - 1;
        int i = 0;
        while ((i<params.size()) && (position>=0))
        {
             position = position - params[i].length() - 1;
             i++;
        };
        location = i;
    }
    
    
    
    quickStrip(*snippetContent,'\r');
    quickStrip(*snippetContent,'\n');
    
    
    updateScintilla(0,g_customSciHandle);
    
    ::SendScintilla(SCI_SETTEXT,0,(LPARAM)*snippetContent);
    ::SendScintilla(SCI_GOTOPOS,0,0);
    
    std::vector<std::string> spotVector;
    
    int endPos;
    int startPos;
    int i = g_listLength-1;
    
    do
    {
        do
        {
            endPos = -1;
            startPos = -1;
    
            endPos = searchNext(g_tagTailList[i]);
            if (endPos >= 0)
            {
                endPos = endPos+3;
                startPos = searchPrevMatchedSign(g_tagSignList[i],g_tagTailList[i]);
            }
            
            if ((endPos != -1) && (startPos != -1))
            {
                
                char* getText = new char[endPos-startPos+1];
                sciGetText(&getText,startPos,endPos);
                char* getText2 = new char[endPos-3-(startPos+4)+1];
                sciGetText(&getText2,startPos+4,endPos-3);
    
    
                if ((getText2[0]=='(') && (getText2[4]==')'))
                {
                } else
                {
                    ::SendScintilla(SCI_SETSEL,startPos,endPos);
                    int found = -1;
                    do
                    {
                        ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                        found = searchNext(getText);
                    } while (found >= 0);
    
                    endPos = startPos;
                    spotVector.push_back(getText2);
                }
    
                delete [] getText;
                delete [] getText2;
            }
    
            ::SendScintilla(SCI_GOTOPOS,endPos,0);
            
        } while ((endPos != -1) && (startPos != -1));
        i--;
    } while (i>=0);
    
    ::SendScintilla(SCI_SETTEXT,0,(LPARAM)"");
    ::SendScintilla(SCI_EMPTYUNDOBUFFER,0,0);
    
    updateScintilla();
    
    std::vector<std::string> hintTextVector;
    hintTextVector.push_back("");
    hintTextVector.push_back("");
    hintTextVector.push_back("");
    
    int cell = 0;
    
    if (location == 0) cell++;
    
    hintTextVector[cell] = *tempTriggerText;
    
    if (spotVector.size()!=0) 
    {
        if (cell == 1) cell++;
        hintTextVector[cell]=hintTextVector[cell]+"(";
        int j =0;
        for (j=0;j< spotVector.size();j++)
        {
            if (spotVector[j].length()==0) spotVector[j] = "{empty}";
    
            if (cell == 1) cell++;
            
            if (j!=0) hintTextVector[cell] = hintTextVector[cell] + ",";
    
            if ((location==j+1) && (cell!=1)) cell++;
            
            hintTextVector[cell] = hintTextVector[cell] + spotVector[j];
        }
        if (cell == 1) cell++;
    
        hintTextVector[cell]=hintTextVector[cell]+")"; //+toString(location);
    }
    return hintTextVector;
   
}

void insertRegularHotSpotSign()
{
    insertTagSign(0);
}
void insertKeyWordSign()
{
    insertTagSign(1);
}
void insertChainSnippetSign()
{
    insertTagSign(2);
}
void insertCommandSign()
{
    insertTagSign(3);
}
void insertOptionSign()
{
    insertTagSign(4);
}
void insertListSign()
{
    insertTagSign(5);
}

void insertEndSign()
{
    insertTagSign(6);
}

//void insertWarmSpotSign()
//{
//    insertTagSign("${!{}!}");
//}


void insertTagSign(int type)
{

    if (g_editorView)
    {
        
        int posStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
        int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION, posStart, 0);

        if (lineCurrent<3) ::SendScintilla(SCI_GOTOLINE,3,0);
        

        int start = -1;
        int end = -1;

        switch (type)
        {
            case 0:
            {
                //TODO: Fingertext should check whether a final caret position exists or not
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[0[]0]");
                start = posStart;
                end = posStart;
                break;
            }

            case 1:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![default]!]");
                start = posStart + 4;
                end = posStart + 11;
                break;
            }

            case 2:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(key)SOMEKEYWORD]!]");
                start = posStart + 9;
                end = posStart + 20;
                break;
            }

            case 3:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(cha)snippetname]!]");
                start = posStart + 9;
                end = posStart + 20;
                break;

            }

            case 4:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(run)somecommand]!]");
                start = posStart + 9;
                end = posStart + 20;
                break;

            }
            case 5:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(opt)option1|option2|option3]!]");
                start = posStart + 9;
                end = posStart + 13;
                break;

            }
            case 6:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(lis)listitem1|listitem2|listitem3]!]");
                start = posStart + 9;
                end = posStart + 18;
                break;

            }
            case 7:
            {
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"$[![(eva)1+2-3*4/5]!]");
                start = posStart + 9;
                end = posStart + 18;
                break;
            }
            case 8:
            {
                //TODO: Fingertext should check whether [>END<] exists or not, one solution is to put an error message if found. Another is to find and replace all the existing [>END<] by "" before adding the new [>END<]

                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"[>END<]");
                start = posStart;
                end = posStart;
                break;

            }

        }
        if (start>=0) ::SendScintilla(SCI_SETSEL,start,end);
         
        //else
        //{
        //    ::showMessageBox(TEXT("Hotspots can be inserted into the content of a snippet only."));
        //
        //}
        //
    } else
    {
        ::showMessageBox(TEXT("Hotspots can be inserted only when you are editing snippets."));
    }

    
}

bool replaceTag(char *expanded, int &posCurrent, int &posBeforeTag)
{
    
    char *expanded_eolfix;
    int eolmode = ::SendScintilla(SCI_GETEOLMODE, 0, 0);
    char *eol[3] = {"\r\n","\r","\n"};
    expanded_eolfix = replaceAll(expanded, "\n", eol[eolmode]);

    int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION, posCurrent, 0);
    int initialIndent = ::SendScintilla(SCI_GETLINEINDENTATION, lineCurrent, 0);

    ::SendScintilla(SCI_INSERTTEXT, posCurrent, (LPARAM)"____`[SnippetInserting]");

	::SendScintilla(SCI_SETTARGETSTART, posBeforeTag, 0);
	::SendScintilla(SCI_SETTARGETEND, posCurrent, 0);
    //::SendScintilla(SCI_REPLACETARGET, strlen(expanded), reinterpret_cast<LPARAM>(expanded));
    ::SendScintilla(SCI_REPLACETARGET, strlen(expanded_eolfix), reinterpret_cast<LPARAM>(expanded_eolfix));

    delete [] expanded_eolfix;
       

     //alert(g_stopCharArray);

    ::SendScintilla(SCI_GOTOPOS,posBeforeTag,0);
    searchNext("`[SnippetInserting]");
    int posEndOfInsertedText = ::SendScintilla(SCI_GETCURRENTPOS,0,0)+19;

    // adjust indentation according to initial indentation
    if (pc.configInt[INDENT_REFERENCE]==1)
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

    ::SendScintilla(SCI_GOTOPOS,posBeforeTag,0);
    //int stopFound = searchNext(g_stopCharArray);
    int stopFound = searchNext("\\$\\[.\\[",true);
    //int stopFound = -1;
    
    if (stopFound<posBeforeTag)
    {
        ::SendScintilla(SCI_GOTOPOS,posEndOfSnippet,0);
        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)g_stopCharArray);
    }
    

    //delete [] g_stopCharArray;

    return true;
}


int getCurrentTag(int posCurrent, char **buffer, int triggerLength)
{
	int length = -1;

    int posBeforeTag;
    if (triggerLength<=0)
    {
        ::SendScintilla(SCI_SETWORDCHARS, 0, (LPARAM)escapeWordChar);
	    posBeforeTag = static_cast<int>(::SendScintilla(SCI_WORDSTARTPOSITION, posCurrent, 1));
        ::SendScintilla(SCI_SETCHARSDEFAULT, 0, 0);
    } else
    {
        posBeforeTag = posCurrent - triggerLength;
    }
                
    if (posCurrent - posBeforeTag < 100) // Max tag length 100
    {
        sciGetText(&*buffer, posBeforeTag, posCurrent);

		length = (posCurrent - posBeforeTag);
	}
    
	return length;
}

void showInsertionDlg()
{
    insertionDlg.doDialog(pc.configInt[INSERTION_DIALOG_STATE]);
}

void showSettingDlg()
{
    settingDlg.doDialog();
}

void showCreationDlg()
{
    
    creationDlg.doDialog();
}




void setInsertionDialogState(int state)
{
    pc.configInt[INSERTION_DIALOG_STATE] = state;
    pc.callWriteConfigInt(INSERTION_DIALOG_STATE);
}

//void setSnippetDockState(int state)
//{
//    pc.configInt[SNIPPETDOCK_STATE] = state;
//    pc.callWriteConfigInt(SNIPPETDOCK_STATE);
//}



void showSnippetDock()
{

    updateMode();

    if (g_enable)
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
	    	data.dlgID = g_snippetDockIndex;
	    	::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);

            //This determine the initial state of the snippetdock.
            //snippetDock.switchDock((bool)pc.configInt[SNIPPETDOCK_STATE]);
            snippetDock.switchDock(true);
            snippetDock.switchInsertMode(pc.configInt[INSERT_MODE]);
            snippetDock.display();
            
	    } else
        {
            snippetDock.display(!snippetDock.isVisible());
        }


        updateMode();
        updateDockItems(true,false,"%",true);
        //snippetHintUpdate();
    } else
    {
        snippetDock.display(false);
        showMessageBox(TEXT("You can open SnippetDock only when Fingertext is enabled."));
    }
    
}
void writeInsertMode(bool insert)
{
    pc.configInt[INSERT_MODE] = insert;
    pc.callWriteConfigInt(INSERT_MODE);
    
}

bool snippetHintUpdate()
{     
    if ((!g_editorView) && (pc.configInt[LIVE_HINT_UPDATE] == 1) && (g_rectSelection == false) && (g_optionMode == false))
    {
        if (snippetDock.isVisible())
        {
            pc.configInt[LIVE_HINT_UPDATE]=0;
            //HWND curScintilla = getCurrentScintilla();
            //if ((::SendScintilla(SCI_GETMODIFY,0,0)!=0) && (::SendScintilla(SCI_SELECTIONISRECTANGLE,0,0)==0))
            if (::SendScintilla(SCI_SELECTIONISRECTANGLE,0,0) == 0)
            {
                
                int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                char *partialTag;
	            int tagLength = getCurrentTag(posCurrent, &partialTag);
                
                if (tagLength==0)
                {
                    updateDockItems(true,false,"%",true);
                } else if ((tagLength>0) && (tagLength<20))
                {
                    //alertNumber(tagLength);
                    char similarTag[MAX_PATH]="";
                    if (pc.configInt[INCLUSIVE_TRIGGERTEXT_COMPLETION]==1) strcat(similarTag,"%");
                    strcat(similarTag,partialTag);
                    strcat(similarTag,"%");
            
                    updateDockItems(true,false,similarTag,true);
                }
                
                if (tagLength>=0) delete [] partialTag;   
            }
            pc.configInt[LIVE_HINT_UPDATE]=1;
        }
        return true;
    } else
    {
        return false;
    }

    //if (g_modifyResponse) refreshAnnotation();
}

void updateDockItems(bool withContent, bool withAll, char* tag, bool populate, bool populateInsertion,bool searchType)
{   
    if (!g_freezeDock)
    {
        pc.configInt[LIVE_HINT_UPDATE]--;

        g_snippetCache.clear();
        
        sqlite3_stmt *stmt;

        if (g_editorView) withAll = true;
        
        int sqlitePrepare;
        char* sqlite3Statement = new char[400];
        
        const char* selectClause1 = "SELECT tag,tagType,snippet FROM snippets ";
        const char* selectClause2 = "SELECT tag,tagType FROM snippets ";
        const char* orderClause1 = "ORDER BY tagType DESC,tag DESC ";
        const char* orderClause2 = "ORDER BY tag DESC,tagType DESC ";
        //const char* whereClause = "WHERE (tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ? OR tagType LIKE ?) AND tag LIKE ? ";
        const char* whereClause = "WHERE tagType LIKE ? AND tag LIKE ? ";
        const char* whereClause2 = "WHERE tagType LIKE ? OR tag LIKE ? ";


        strcpy(sqlite3Statement,"");
        if (withContent) strcat(sqlite3Statement,selectClause1);
        else strcat(sqlite3Statement,selectClause2);
        if (!searchType) strcat(sqlite3Statement, whereClause);
        else strcat(sqlite3Statement, whereClause2);
        if (pc.configInt[SNIPPET_LIST_ORDER_TAG_TYPE]==1) strcat(sqlite3Statement, orderClause1);
        else strcat(sqlite3Statement, orderClause2);

        sqlitePrepare = sqlite3_prepare_v2(g_db, sqlite3Statement, -1, &stmt, NULL);
        
	    if (g_dbOpen && SQLITE_OK == sqlitePrepare)
	    {
            std::vector<std::string> scopeList = generateScopeList();

            int i=scopeList.size()-1;
            do
            {

                if (withAll)
                {
                    
                    if (searchType)
                    {
                        sqlite3_bind_text(stmt, 1, tag, -1, SQLITE_STATIC);
                        
                    } else
                    {
                        sqlite3_bind_text(stmt, 1, "%" , -1, SQLITE_STATIC);
                    }

                    sqlite3_bind_text(stmt, 2, tag, -1, SQLITE_STATIC);

                } else
                {   
                    const char* scopeListItem = scopeList[i].c_str();
                    sqlite3_bind_text(stmt, 1, scopeListItem, -1, SQLITE_STATIC);
                    sqlite3_bind_text(stmt, 2, tag, -1, SQLITE_STATIC);
                }
                
                while(true)
                {

                    if(SQLITE_ROW == sqlite3_step(stmt))
                    {
                        
                        SnipIndex tempSnipIndex;
                        tempSnipIndex.triggerText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                        tempSnipIndex.scope = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                        if (withContent) tempSnipIndex.content = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                        
                        g_snippetCache.push_back(tempSnipIndex);
                    }
                    else
                    {
                        break;  
                    }
                }
                sqlite3_reset(stmt);

                i--;
                //alert(i);
            } while ((i>=0) && (!withAll));
            //} while ((i < nameParts.size()) && (!withAll));
        }
        sqlite3_finalize(stmt);
        delete [] sqlite3Statement;


        if (populateInsertion)
        {
            populateDockItems(withAll,true);
            showPreview(true,true);
        } else if (populate)
        {
            populateDockItems(withAll);
            if (pc.configInt[LIVE_PREVIEW_BOX]==1) showPreview(true,false);
        }
        
        pc.configInt[LIVE_HINT_UPDATE]++;
        //::SendMessage(getCurrentScintilla(),SCI_GRABFOCUS,0,0);   
    }
    
    
}


void updateInsertionDialogHint()
{
    insertionDlg.updateInsertionHint();
}

wchar_t* constructDockItems(std::string scope, std::string triggerText, int maxlength)
{
    std::string newText = "<"+scope+">";
    int scopeLength;
    scopeLength = maxlength - newText.length();
    if (scopeLength < 3) scopeLength = 3;
    for (int i=0;i<scopeLength;i++) newText = newText + " ";
    newText = newText + triggerText;
    return toWideChar(newText);


}

void populateDockItems(bool withAll, bool insertion)
{
    if (insertion)
    {
        insertionDlg.clearList();
    } else
    {
        snippetDock.clearDock();
    }

    //for (int j=0;j<pc.configInt[SNIPPET_LIST_LENGTH];j++)
    int j=0;
    for (j=0;j<g_snippetCache.size();j++)
    {
        if ((withAll) || (g_snippetCache[j].triggerText[0] != '_'))
        {   
            int maxLength = 14;
            if (insertion) maxLength = 12;

            wchar_t* convertedTagText = constructDockItems(g_snippetCache[j].scope,g_snippetCache[j].triggerText,maxLength);

            if (insertion) 
            {
                insertionDlg.addDockItem(convertedTagText);
            } else
            {
                snippetDock.addDockItem(convertedTagText);
            }

            delete [] convertedTagText;

        }
    }
    

    if (!insertion)
    {
        wchar_t* countText = toWideChar(toString(j)+"/"+g_snippetCount);
        snippetDock.updateSnippetCount(countText);
        delete [] countText;
    }

    
    //deleteCache();
}

void updateSnippetCount()
{
    sqlite3_stmt *stmt;

    const char *sqlitePrepareStatement = "SELECT COUNT(*) FROM snippets";
    
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, sqlitePrepareStatement, -1, &stmt, NULL))
	{
        if(SQLITE_ROW == sqlite3_step(stmt)) 
		{
			g_snippetCount = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
		}
	}
    
	sqlite3_finalize(stmt);

}

void setTextTarget(bool fromTab)
{
    insertionDlg.setTextTarget(fromTab);
}

void setListTarget()
{    
    insertionDlg.setListTarget();
}

void installDefaultPackage()
{
    updateSnippetCount();
    
    int messageReturn = IDYES;

    if (toDouble(g_snippetCount) != 0)
    {
        messageReturn = showMessageBox(TEXT("It seems that you already have some snippets in your FingerText database.\r\n\r\nAre you sure that you want to install the Default Snippet Package?"),MB_YESNO);
        if (messageReturn == IDNO) return;
    }
    

    HGLOBAL     res_handle = NULL;
    HRSRC       res;
    char *      res_data;
    DWORD       res_size;

    // NOTE: providing g_hInstance is important, NULL might not work
    res = FindResource((HINSTANCE)g_hModule, MAKEINTRESOURCE(SNIPPET_RESOURCE), RT_RCDATA);
    //res = FindResource(NULL, MAKEINTRESOURCE(MY_RESOURCE), RT_RCDATA);
    
    if (!res)
    {
        showMessageBox(TEXT("Error importing Default Snippets."));
        return;
    }
    
    res_handle = LoadResource((HINSTANCE)g_hModule, res);
    if (!res_handle)
    {
        showMessageBox(TEXT("Error importing Default Snippets."));
        return;
    }
    
    res_data = (char*)LockResource(res_handle);
    res_size = SizeofResource(NULL, res);

    std::string defaultSnippetText;
    defaultSnippetText = toString(res_data);
    
    std::ofstream myfile;
    myfile.open (g_downloadPath);
    //myfile << getDefaultPackage();
    myfile << defaultSnippetText;
    myfile.close();
    importSnippets(g_downloadPath);

    //delete [] res_data;
}

void downloadDefaultPackage()
{
    TCHAR* server = new TCHAR[MAX_PATH];
    TCHAR* request = new TCHAR[MAX_PATH];
    TCHAR* type = new TCHAR[MAX_PATH];
    TCHAR* path = new TCHAR[MAX_PATH];

    ::wcscpy(server,TEXT("cloud.github.com"));
    ::wcscpy(request,TEXT("/downloads/erinata/FingerText/FingerText0.5.58SampleSnippets.ftd"));
   
    ::wcscpy(type,TEXT("GET"));
    ::wcscpy(path,g_downloadPath);

    ::SendScintilla(SCI_SETCURSOR, SC_CURSORWAIT, 0);
    httpToFile(server,request,type,path);
    ::SendScintilla(SCI_SETCURSOR, SC_CURSORNORMAL, 0);

    importSnippets(g_downloadPath);

    delete [] server;
    delete [] request;
    delete [] type;
    delete [] path;


}

bool backupAllSnippets()
{
    TCHAR* backupPath = new TCHAR[MAX_PATH];
    ::wcscpy(backupPath,g_basePath);
    ::wcscat(backupPath,TEXT("\\SnippetsBackup-"));
        
    char* dateText = getDateTime("yyyyMMdd");
    char* timeText = getDateTime("HHmm",false);
    TCHAR* dateTextWide;
    TCHAR* timeTextWide;
    dateTextWide = toWideChar(dateText);
    timeTextWide = toWideChar(timeText);
    
    ::wcscat(backupPath,dateTextWide);
    ::wcscat(backupPath,timeTextWide);
    ::wcscat(backupPath,TEXT(".ftd"));
    
    bool retVal = exportSnippets(true, backupPath);

    delete [] backupPath;
    delete [] dateText;
    delete [] timeText;
    delete [] dateTextWide;
    delete [] timeTextWide;
    return retVal;
}


void exportAndClearSnippets()
{
    
    int messageReturn = showMessageBox(TEXT("Are you sure that you want to clear the whole snippet database?"),MB_YESNO);
    //int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("Are you sure that you want to clear the whole snippet database?"), TEXT(PLUGIN_NAME), MB_YESNO);
    if (messageReturn == IDYES)
    {
        if (backupAllSnippets())
        {
            clearAllSnippets();
            showMessageBox(TEXT("All snippets are deleted. \r\n\r\nIf you want to UNDO this action now, you can recover your snippets by importing the SnippetsBackup.ftd file in the Fingertext config folder."));
        } else
        {
            showMessageBox(TEXT("An error occured. The snippet database cannot be cleared. If you really want to clear the snippet database you can close Notepad++ and remove the FingerText.db3 in the config folder."));
        }
        
    } else 
    {
        showMessageBox(TEXT("Snippet clearing is aborted."));
        
    }
    
}

void exportSnippetsOnly()
{
    exportSnippets();
}

void clearAllSnippets()
{
    sqlite3_stmt *stmt;
                
    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets", -1, &stmt, NULL)) sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "VACUUM", -1, &stmt, NULL)) sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    updateSnippetCount();
    updateDockItems(true,false,"%",true);
}

bool exportSnippets(bool all, wchar_t* path)
{
    ////TODO: Can actually add some informtiaon at the end of the exported snippets......can be useful information like version number or just describing the package
    
    pc.configInt[LIVE_HINT_UPDATE]--;  // Temporary turn off live update as it disturb exporting
    //

    //TODO: improve efficiency by ignoring these in the case of withPath
    bool success = false;
    
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = TEXT("");
    ZeroMemory(&ofn, sizeof(ofn));
    
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("FingerText Datafiles (*.ftd)\0*.ftd\0");
    ofn.lpstrFile = (LPWSTR)fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("");

    bool getSave = false;
    bool withPath = true;
    if (::wcscmp(path,TEXT(""))==0) withPath = false;
    if (!withPath) getSave = ::GetSaveFileName(&ofn);

    if ((getSave) || (withPath))
    {
        if (all) updateDockItems(true,true,"%",false);
        g_freezeDock = true;
        
        ::SendScintilla(SCI_SETCURSOR, SC_CURSORWAIT, 0);
        //pc.configInt[SNIPPET_LIST_LENGTH] = 100000;
        //g_snippetCache = new SnipIndex [pc.configInt[SNIPPET_LIST_LENGTH]];
        //updateDockItems(true,false,"test%",false);  // This is for exporting a subset of the dataset
        
        
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
        LRESULT importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
        ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);


        int exportCount = 0;
        //for (int j=0;j<pc.configInt[SNIPPET_LIST_LENGTH];j++)
        for (int j=0;j<g_snippetCache.size();j++)
        {
            if (g_snippetCache[j].scope != "")
            {
                char* triggerText = toCharArray(g_snippetCache[j].triggerText);
                char* scope = toCharArray(g_snippetCache[j].scope);
                char* content = toCharArray(g_snippetCache[j].content);

                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)triggerText);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\n");
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)scope);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\n");
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)content);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"!$[FingerTextData FingerTextData]@#\r\n");
                exportCount++;
                delete [] triggerText;
                delete [] scope;
                delete [] content;
            }
        }

        ::SendScintilla(SCI_CONVERTEOLS,SC_EOL_LF, 0);
        if (withPath)
        {
            ::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILEAS, 0, (LPARAM)path);
        } else
        {
            ::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTFILEAS, 0, (LPARAM)fileName);
        }
        
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

        if (!withPath) showMessageBox(exportCountText);

        g_freezeDock = false;
        //::MessageBox(nppData._nppHandle, exportCountText, TEXT(PLUGIN_NAME), MB_OK);
    }
    //pc.configInt[SNIPPET_LIST_LENGTH] = GetPrivateProfileInt(TEXT(PLUGIN_NAME), TEXT("snippet_list_length"), 1000 , pc.iniPath); // TODO: This hard coding of DEFAULT_SNIPPET_LIST_LENGTH is temporary and can cause problem.
    //g_snippetCache = new SnipIndex [pc.configInt[SNIPPET_LIST_LENGTH]];
    
    pc.configInt[LIVE_HINT_UPDATE]++;
    
    
    updateDockItems(true,false,"%",true);

    return success;
    
}

void importSnippetsOnly()
{
    importSnippets();
}


//TODO: importsnippet and savesnippets need refactoring sooooo badly
//TODO: Or it should be rewrite, import snippet should open the snippetediting.ftb, turn or annotation, and cut and paste the snippet on to that file and use the saveSnippet function
void importSnippets(wchar_t* path)
{
    
    
    //TODO: importing snippet will change the current directory, which is not desirable effect
    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_ftbPath))
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
        //TODO: prompt for closing tab instead of just warning
        //showMessageBox(TEXT("Please close all the snippet editing tabs (SnippetEditor.ftb) before importing any snippet pack."));
        //::MessageBox(nppData._nppHandle, TEXT("Please close all the snippet editing tabs (SnippetEditor.ftb) before importing any snippet pack."), TEXT(PLUGIN_NAME), MB_OK);
        //return;
    }

    if (::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)g_fttempPath))
    {
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_SAVE);
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
    }   


    //TODO: verify why an error may occur when this backupAllSnippet block is put inside the ((getSave) || (withPath)) block
    //if (backupAllSnippets())
    //{
    //    showMessageBox(TEXT("Fingertext backed up your current snippet database in the SnippetBackup.ftd in the config folder.\r\n\r\nIn case any thing strange happened during the import process, you can recover you snippets by importing the backup ftd file."),MB_OK);
    //} else
    //{
    //    int continueImport = showMessageBox(TEXT("An error occurred and Fingertext cannot backup your current snippet database. \r\n\r\nYou can continue to import the snippets but you may not be able to undo this action.\r\n\r\n Are you sure that you want to contine with the import action?"),MB_YESNO);
    //    if (!(continueImport == IDYES)) return;
    //}
    //
    //TODO: to decide on whether to silently backup or to explicitly tell the user the snippets are backup.
    if (toDouble(g_snippetCount) != 0) backupAllSnippets();


    g_freezeDock = true;
    pc.configInt[LIVE_HINT_UPDATE]--;
    
    //TODO: improve efficiency by ignoring these in the case of withPath
    OPENFILENAME ofn;
    wchar_t fileName[MAX_PATH] = TEXT("");
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = TEXT("FingerText Datafiles (*.ftd)\0*.ftd\0");
    ofn.lpstrFile = (LPWSTR)fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = TEXT("");
    
    bool getSave = false;
    bool withPath = true;
    if (::wcscmp(path,TEXT(""))==0) withPath = false;
    if (!withPath) getSave = ::GetSaveFileName(&ofn);

    if ((getSave) || (withPath))
    {   
        int conflictKeepCopy = IDNO;
        if (toDouble(g_snippetCount) != 0)
        {
            conflictKeepCopy = showMessageBox(TEXT("Do you want to keep both versions if the imported snippets are conflicting with existing one?\r\n\r\nYes - Keep both versions\r\nNo - Overwrite existing version\r\nCancel - Stop importing"),MB_YESNOCANCEL);
        }

        if (conflictKeepCopy == IDCANCEL)
        {
            showMessageBox(TEXT("Snippet importing aborted."));
            return;
        }

        
        //::MessageBox(nppData._nppHandle, (LPCWSTR)fileName, TEXT(PLUGIN_NAME), MB_OK);
        std::ifstream file;
        //file.open((LPCWSTR)fileName, std::ios::binary | std::ios::in);     //TODO: verified why this doesn't work. Specifying the binary thing will cause redundant copy keeping when importing
        
        if (withPath)
        {
            file.open((LPCWSTR)path); 
        } else
        {
            file.open((LPCWSTR)fileName); 
        }
        

        file.seekg(0, std::ios::end);
        int fileLength = file.tellg();
        file.seekg(0, std::ios::beg);

        if (file.is_open())
        {
            char* fileText = new char[fileLength+1];
            ZeroMemory(fileText,fileLength);

            file.read(fileText,fileLength);
            fileText[fileLength] = '\0';
            file.close();
        
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
            LRESULT importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
            ::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);

            //HWND curScintilla = getCurrentScintilla();
            ::SendScintilla(SCI_SETCURSOR, SC_CURSORWAIT, 0);

            ::SendScintilla(SCI_CONVERTEOLS,SC_EOL_LF, 0);

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
                
                //::SendScintilla(SCI_SETSELECTION,snippetPosStart,snippetPosEnd);
                //snippetText = new char[snippetPosEnd-snippetPosStart + 1];
                //::SendScintilla(SCI_GETSELTEXT, 0, reinterpret_cast<LPARAM>(snippetText));
                sciGetText(&snippetText,snippetPosStart,snippetPosEnd);


                ::SendScintilla(SCI_SETSELECTION,0,snippetPosEnd+1); // This +1 corrupt the ! in !$[FingerTextData FingerTextData]@# so that the program know a snippet is finished importing
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");

                sqlite3_stmt *stmt;
                
                notOverWrite = false;
                
                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "SELECT snippet FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt, NULL))
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
                        
                        //snippetTextOldCleaned = quickStrip(snippetTextOld,'\r');
                        snippetTextOld = quickStrip(snippetTextOld,'\r');

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
                                if (pc.configInt[IMPORT_OVERWRITE_CONFIRM] == 1)
                                {
                                    // TODO: may be moving the message to earlier location so that the text editor will be showing the message that is about to be overwriting into the database
                                    // TODO: try showing the conflict message on the editor
                        
                                    ::SendScintilla(SCI_GOTOLINE,0,0);
                                    //TODO: refactor this repeated replacesel action
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\nConflicting Snippet: \n\n     ");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tagText);
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"  <");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)tagTypeText);
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)">\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\n\n   (More details of the conflicts will be shown in future releases)");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"\n\n\n\n\n\n\n\n----------------------------------------\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"---------- [ Pending Imports ] ---------\n");
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"----------------------------------------\n");
                        
                                    int messageReturn = showMessageBox(TEXT("A snippet already exists, overwrite?"),MB_YESNO);
                                    //int messageReturn = ::MessageBox(nppData._nppHandle, TEXT("A snippet already exists, overwrite?"), TEXT(PLUGIN_NAME), MB_YESNO);
                                    if (messageReturn==IDNO)
                                    {
                                        //delete [] tagText;
                                        //delete [] tagTypeText;
                                        //delete [] snippetText;
                                        // not overwrite
                                        //::MessageBox(nppData._nppHandle, TEXT("The Snippet is not saved."), TEXT(PLUGIN_NAME), MB_OK);
                                        notOverWrite = true;
                                    } else
                                    {
                                        sqlite3_stmt *stmt2;
                                        // delete existing entry
                                        if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt2, NULL))
                                        {
                                            sqlite3_bind_text(stmt2, 1, tagTypeText, -1, SQLITE_STATIC);
                                            sqlite3_bind_text(stmt2, 2, tagText, -1, SQLITE_STATIC);
                                            sqlite3_step(stmt2);
                                        } else
                                        {
                                            showMessageBox(TEXT("Cannot write into database."));
                                            //::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT(PLUGIN_NAME), MB_OK);
                                        }
                                        sqlite3_finalize(stmt2);
                        
                                    }
                                    ::SendScintilla(SCI_GOTOLINE,17,0);
                                    ::SendScintilla(SCI_SETSELECTION,0,::SendScintilla(SCI_GETCURRENTPOS,0,0));
                                    ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                        
                                } else
                                {
                                    sqlite3_stmt *stmt2;
                                    // delete existing entry
                                    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt2, NULL))
                                    {
                                        sqlite3_bind_text(stmt2, 1, tagTypeText, -1, SQLITE_STATIC);
                                        sqlite3_bind_text(stmt2, 2, tagText, -1, SQLITE_STATIC);
                                        sqlite3_step(stmt2);
                                    } else
                                    {
                                        showMessageBox(TEXT("Cannot write into database."));
                                        //::MessageBox(nppData._nppHandle, TEXT("Cannot write into database."), TEXT(PLUGIN_NAME), MB_OK);
                                    }
                                    sqlite3_finalize(stmt2);
                                }
                            } else
                            {
                                notOverWrite = true;
                                //Delete the old entry
                                sqlite3_stmt *stmt2;
                                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "DELETE FROM snippets WHERE tagType LIKE ? AND tag LIKE ?", -1, &stmt2, NULL))
                                {
                                    sqlite3_bind_text(stmt2, 1, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 2, tagText, -1, SQLITE_STATIC);
                                    sqlite3_step(stmt2);
                                }
                                sqlite3_finalize(stmt2);

                                //write the new entry
                                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?,?)", -1, &stmt2, NULL))
                                {
                                    sqlite3_bind_text(stmt2, 1, tagText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 2, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 3, snippetText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 4, "", -1, SQLITE_STATIC);
                                
                                    sqlite3_step(stmt2);
                                }
                                sqlite3_finalize(stmt2);

                                //write the old entry back with conflict suffix
                                if (SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?,?)", -1, &stmt2, NULL))
                                {
                                    importCount++;
                                    //TODO: add file name to the stamp
                                    char* dateText = getDateTime("yyyyMMdd");
                                    char* timeText = getDateTime("HHmmss",false);
                                    char* tagTextsuffixed;
                                    tagTextsuffixed = new char [strlen(tagText)+256];
                                
                                    strcpy(tagTextsuffixed,tagText);
                                    strcat(tagTextsuffixed,"_OldCopy");
                                    strcat(tagTextsuffixed,dateText);
                                    strcat(tagTextsuffixed,timeText);
                                    
                                    sqlite3_bind_text(stmt2, 1, tagTextsuffixed, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 2, tagTypeText, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 3, snippetTextOld, -1, SQLITE_STATIC);
                                    sqlite3_bind_text(stmt2, 4, "", -1, SQLITE_STATIC);

                                    sqlite3_step(stmt2);


                                    //sqlite3_finalize(stmt);
                                    conflictCount++;
                                    delete [] tagTextsuffixed;
                                    delete [] dateText;
                                    delete [] timeText;
                                }
                                sqlite3_finalize(stmt2);
                            }
                        }
                        delete [] snippetTextOld;
                    }
                }
                sqlite3_finalize(stmt);

                if (notOverWrite == false)
                {
                    if (SQLITE_OK == sqlite3_prepare_v2(g_db, "INSERT INTO snippets VALUES(?,?,?,?)", -1, &stmt, NULL))
                    {
                        importCount++;
                        // Then bind the two ? parameters in the SQLite SQL to the real parameter values
                        sqlite3_bind_text(stmt, 1, tagText, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 2, tagTypeText, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 3, snippetText, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 4, "", -1, SQLITE_STATIC);

                        // Run the query with sqlite3_step
                        sqlite3_step(stmt); // SQLITE_ROW 100 sqlite3_step() has another row ready
                        //::MessageBox(nppData._nppHandle, TEXT("The Snippet is saved."), TEXT(PLUGIN_NAME), MB_OK);
                    }
                    sqlite3_finalize(stmt);
                }
                
                //delete [] tagText;
                //delete [] tagTypeText;
                //delete [] snippetText;
            
                ::SendScintilla(SCI_SETSAVEPOINT,0,0);
                
                updateDockItems(true,false,"%",true);
            
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
                wcscat_s(importCountText,TEXT("\n\nThere are some conflicts between the imported and existing snippets. You may go to the snippet editor to clean them up."));
            }
            //::MessageBox(nppData._nppHandle, TEXT("Complete importing snippets"), TEXT(PLUGIN_NAME), MB_OK);
            showMessageBox(importCountText);
            //::MessageBox(nppData._nppHandle, importCountText, TEXT(PLUGIN_NAME), MB_OK);
            
            ::SendScintilla(SCI_SETSAVEPOINT,0,0);
            ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);

        }
        //delete [] fileText;
    }
    pc.configInt[LIVE_HINT_UPDATE]++;
    g_freezeDock = false;
    updateSnippetCount();
    ::snippetHintUpdate();
    //updateDockItems(true,false,"%",true,false,false);

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
            messageReturn = showMessageBox(message,MB_YESNO);
            //messageReturn=::MessageBox(nppData._nppHandle, message, TEXT(PLUGIN_NAME), MB_YESNO);
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

void updateMode()
{
    updateScintilla();
    g_lastTriggerPosition = 0;
    //TODO: should change to edit mode and normal mode by a button, and dynamically adjust the dock content
    //HWND curScintilla = getCurrentScintilla();
    TCHAR fileType[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileType);
        
    if (::_tcscmp(fileType,TEXT("SnippetEditor.ftb"))==0)
    {
        snippetDock.toggleSave(true);
        g_editorView = true;
        if (snippetDock.isVisible()) snippetDock.switchDock(false);
        //snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("EDIT MODE\r\n(Double click item in list to edit another snippet, Ctrl+S to save)"));
        updateLineCount();
    } else if (g_enable)
    {
        snippetDock.toggleSave(false);
        g_editorView = false;
        if (snippetDock.isVisible()) snippetDock.switchDock(true);
        //snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Enabled]\r\n(Type trigger text and hit tab to insert snippet)"));
    } else
    {
        snippetDock.toggleSave(false);
        g_editorView = false;

        if (snippetDock.isVisible()) snippetDock.switchDock(true);
        //snippetDock.setDlgText(IDC_LIST_TITLE, TEXT("NORMAL MODE [FingerText Disabled]\r\n(To enable: Plugins>FingerText>Toggle FingerText On/Off)"));
    }
}

void showSettings()
{
    pc.settings();
}

void showHelp()
{
    pc.help();
}

void showAbout()
{
    pc.about();
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
__________________________________________________________________________\r\n\r\n\r\n\r\n\r\n\
       =============   TriggerText   =============                        ");
            } else if (lineCurrent == 2)
            {
                ::SendScintilla(SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
__________________________________________________________________________\r\n\
 Snippet Editor Hint: \r\n\r\n\
 Scope determines where the snippet is available.\r\n\
 e.g. \"GLOBAL\"(without quotes) for globally available snippets.\r\n\
 \"Ext:cpp\"(without quotes) means available in .cpp documents and\r\n\
 \"Lang:HTML\"(without quotes) for all html documents.\r\n\
__________________________________________________________________________\r\n\r\n\r\n\r\n\
       =============   TriggerText   =============                        ");
            } else
            {
                ::SendScintilla(SCI_ANNOTATIONSETTEXT, 0, (LPARAM)"\
__________________________________________________________________________\r\n\
 Snippet Editor Hint: \r\n\r\n\
 Snippet Content is the text that is inserted to the editor\r\n\
 when a snippet is triggered.\r\n\
 It can be as long as many paragraphs or just several words.\r\n\r\n\
 Place a $[0[]0] to set the final location of the caret.\r\n\
 Remember to place an [>END<] at the very end of the snippet.\r\n\
__________________________________________________________________________\r\n\r\n\
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
    g_optionArray.clear();
    
    //int i = 0;
    //while (i<g_optionNumber)
    //{
    //    //alertNumber(i);
    //    delete [] g_optionArray[i];
    //    i++;
    //};
    turnOffOptionMode();
    //g_optionMode = false;
    //g_optionNumber = 0;
    g_optionCurrent = 0;
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
        if (g_optionCurrent >= g_optionArray.size()-1) 
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
            g_optionCurrent = g_optionArray.size()-1;
        } else
        {
            g_optionCurrent--;
        }
    }
    //return item;
    //return g_optionArray[g_optionCurrent];
}

void recordOptionText()
{
    char* optionText;
    sciGetText(&optionText,-1,-1);
    g_lastOption = toString(optionText);
    delete [] optionText;

}

void recordLastListItem(const char* item)
{
    char* lastListItemCharArray = toCharArray(item);
    g_lastListItem = toString(lastListItemCharArray);
    delete [] lastListItemCharArray;
}

void turnOffOptionMode()
{
    
    if (g_optionMode) recordOptionText();
    g_optionMode = false;
}

void turnOnOptionMode()
{
    
    g_optionMode = true;
}
void optionNavigate(bool toNext)
{
   
    pc.configInt[LIVE_HINT_UPDATE]--;
    //g_selectionMonitor = false;

    ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    //::SendMessage(getCurrentScintilla(),SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    updateOptionCurrent(toNext);
    char* option = toCharArray(g_optionArray[g_optionCurrent]);
    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)option);
    //::SendMessage(getCurrentScintilla(),SCI_REPLACESEL, 0, (LPARAM)option);
    delete [] option;
    //::SendScintilla(SCI_GOTOPOS,g_optionStartPosition,0);
    //::SendMessage(getCurrentScintilla(),SCI_GOTOPOS,g_optionStartPosition,0);
    g_optionEndPosition = g_optionStartPosition + g_optionArray[g_optionCurrent].length();
    ::SendScintilla(SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    //::SendMessage(getCurrentScintilla(),SCI_SETSELECTION,g_optionStartPosition,g_optionEndPosition);
    //g_selectionMonitor = true;
    pc.configInt[LIVE_HINT_UPDATE]++;
   
}

bool withSelection = false;

void selectionMonitor(int contentChange)
{
    //TODO: pasting text with more then one line in the scope field will break editor restriction
    //TODO: lots of optimization needed
    

    //In normal view, this code is going to cater the option navigation. In editor view, it restrict selection in first 3 lines
    if (g_selectionMonitor == 1)
    {
        //TODO: this "100" is associated with the limit of number of multiple hotspots that can be simultaneously activated, should find a way to make this more customizable

        if (::SendScintilla(SCI_GETSELECTIONS,0,0)>100)
        {
            g_rectSelection = true;
        } else
        {
            if (::SendScintilla(SCI_GETSELECTIONMODE,0,0)!=SC_SEL_STREAM)
            {
                g_rectSelection = true;
            } else
            {
                g_rectSelection = false;
            }
        }

        g_modifyResponse = false;
        g_selectionMonitor--;
        if (g_editorView == false)
        {
                        //TODO: use hook to cater option? (so that the bug of empty options can be fixed
            //TODO: reexamine possible performance improvement

            if (contentChange & (SC_UPDATE_SELECTION))
            {
                
                
                if (g_optionMode)
                {
                    int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                    //alertNumber(g_optionStartPosition);
                    //alertNumber(posCurrent);
                    //TODO: a bug when there is an empty option and the hotspot is at the beginning of document
                    if (posCurrent > g_optionStartPosition)
                    {
                        optionNavigate(true);
                        //optionTriggered = true;
                        turnOnOptionMode();
                    } else
                    {
                        optionNavigate(false);
                        turnOnOptionMode();
                    }
                    //else
                    //{
                    //    cleanOptionItem();
                    //    g_optionMode = false;
                    //}
                } else
                {
                    int selectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
                    int selectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);

                    
                    if (selectionStart==selectionEnd)
                    {
                        g_onHotSpot = false;    
                        withSelection = false;
                        
                    } else if ((!withSelection) && (selectionStart!=selectionEnd))
                    {
                        withSelection = true;
                        updateDockItems(true,false,"%",true);
                    } 
                    
                }
                
            }
        } else if (pc.configInt[EDITOR_CARET_BOUND] == 1)
        {
            int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION,posCurrent,0);
            int selectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
            int selectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);
            int selectionStartLine = ::SendScintilla(SCI_LINEFROMPOSITION,selectionStart,0);
            int selectionEndLine = ::SendScintilla(SCI_LINEFROMPOSITION,selectionEnd,0);
            
            //// Failed attempt to use SCI_KEY to restrict caret movement
            //SendScintilla(SCI_ASSIGNCMDKEY,SCK_UP,SCI_LINEUP); 
            //SendScintilla(SCI_ASSIGNCMDKEY,SCK_DOWN,SCI_LINEDOWN); 
            //if ((lineCurrent == 2) || (lineCurrent == 1) || (selectionStartLine==2) || (selectionStartLine==1) || (selectionEndLine==2)|| (selectionEndLine==1))
            //{
            //    SendScintilla(SCI_ASSIGNCMDKEY,SCK_UP,SCI_NULL); 
            //    SendScintilla(SCI_ASSIGNCMDKEY,SCK_DOWN,SCI_NULL); 
            //} else if ((lineCurrent == 3)|| (selectionStartLine==3) || (selectionEndLine==3))
            //{
            //    alert();
            //    SendScintilla(SCI_ASSIGNCMDKEY,SCK_UP,SCI_NULL); 
            //} else if ((lineCurrent == 0)|| (selectionStartLine==0) || (selectionEndLine==0))
            //{
            //    ::SendScintilla(SCI_GOTOLINE,1,0);
            //} 
            //
            int firstlineEnd = ::SendScintilla(SCI_GETLINEENDPOSITION,0,0);
            int currentLineCount = ::SendScintilla(SCI_GETLINECOUNT,0,0);
            
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
                    showMessageBox(TEXT("The TriggerText length limit is 40 characters."));
                    //::MessageBox(nppData._nppHandle, TEXT("The TriggerText length limit is 40 characters."), TEXT(PLUGIN_NAME), MB_OK);
                    ::SendScintilla(SCI_UNDO,0,0);
                    updateLineCount();
                
                } else if (::SendScintilla(SCI_LINELENGTH,2,0)>=251)
                {
                    showMessageBox(TEXT("The Scope length limit is 250 characters."));
                    //::MessageBox(nppData._nppHandle, TEXT("The Scope length limit is 250 characters."), TEXT(PLUGIN_NAME), MB_OK);
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
        refreshAnnotation();  //TODO: consider only refresh annotation under some situation (for example only when an undo is done) to improve efficiency.
    }
}

void doTagComplete()
{
    tagComplete();
}

int tagComplete()
{
    int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
    int index = -1;
    char *tag;
    int tagLength = getCurrentTag(posCurrent, &tag);
    char *key = new char[300];
    strcpy(key,"");
    if (pc.configInt[INCLUSIVE_TRIGGERTEXT_COMPLETION]==1) strcat(key,"%");
    strcat(key,tag);
    strcat(key,"%");

    if (tagLength > 0)
    {
        //if (pc.configInt[LIVE_HINT_UPDATE]<=0) updateDockItems(false,false,key,false);
        updateDockItems(true,false,key,false);
        index = g_snippetCache.size()-1;
        while ((index >= 0) && (g_snippetCache[index].triggerText[0] == '_')) index--;
        if (index >= 0)
        {
            
            SendScintilla(SCI_SETSEL,posCurrent-tagLength,posCurrent);
            char* triggerText;
            triggerText = toCharArray(g_snippetCache[index].triggerText);
            SendScintilla(SCI_REPLACESEL,0,(LPARAM)triggerText);
            delete [] triggerText;

        }

    }
    if (pc.configInt[LIVE_HINT_UPDATE]<=0) updateDockItems(true,false,"%",true);
    return index;
    //if (triggerTag(posCurrent,true) > 0) snippetHintUpdate();
}

void triggerSave()
{
    if (g_editorView) ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_SAVE);
}

std::vector<std::string> getAssociatedScopes(std::string s)
{
    //TODO: move this to the setting
    std::vector<std::string> result;

    if (s.compare("Lang:TXT")==0)
    {
        result.push_back("Ext:txt");
    } else if (s.compare("Lang:PHP")==0)
    {
        result.push_back("Ext:php");
        result.push_back("Ext:php3");
        result.push_back("Ext:phtml");
        result.push_back("Ext:html");
          
    } else if (s.compare("Lang:C")==0)
    {
        result.push_back("Ext:c");
        result.push_back("Ext:h");

    } else if (s.compare("Lang:CPP")==0)
    {
        result.push_back("Ext:cpp");
        result.push_back("Ext:hpp");
        result.push_back("Ext:c");
        result.push_back("Ext:h");
    } else if (s.compare("Lang:CS")==0)
    {
        result.push_back("Ext:cs");

    } else if (s.compare("Lang:OBJC")==0)
    {
        result.push_back("Ext:m");
        result.push_back("Ext:h");
    } else if (s.compare("Lang:JAVA")==0)
    {
        result.push_back("Ext:java");
    } else if (s.compare("Lang:RC")==0)
    {
        result.push_back("Ext:rc");
    } else if (s.compare("Lang:HTML")==0)
    {
        result.push_back("Ext:html");
        result.push_back("Ext:htm");
        result.push_back("Ext:js");
        result.push_back("Ext:css");
    } else if (s.compare("Lang:XML")==0)
    {
        result.push_back("Ext:xml");
        result.push_back("Ext:xsml");
        result.push_back("Ext:xsl");
        result.push_back("Ext:xsd");
        result.push_back("Ext:kml");    
        result.push_back("Ext:wsdl");    
        
    } else if (s.compare("Lang:MAKEFILE")==0)
    {
        result.push_back("Ext:mak");   
        
    } else if (s.compare("Lang:PASCAL")==0)
    {
        result.push_back("Ext:pas");   
        result.push_back("Ext:inc");   

    } else if (s.compare("Lang:BATCH")==0)
    {
        result.push_back("Ext:bat");
        result.push_back("Ext:cmd");
        result.push_back("Ext:nt");
          

    } else if (s.compare("Lang:INI")==0)
    {
        result.push_back("Ext:ini");
        result.push_back("Ext:inf");
        result.push_back("Ext:reg");
        result.push_back("Ext:url");
           
    } else if (s.compare("Lang:NFO")==0)
    {
        result.push_back("Ext:nfo");

    } else if (s.compare("Lang:ASP")==0)
    {
        result.push_back("Ext:asp");
    } else if (s.compare("Lang:SQL")==0)
    {
        result.push_back("Ext:sql");
    } else if (s.compare("Lang:VB")==0)
    {
        result.push_back("Ext:vb");
        result.push_back("Ext:vbs");
    } else if (s.compare("Lang:JS")==0)
    {
        result.push_back("Ext:js");
    } else if (s.compare("Lang:CSS")==0)
    {
        result.push_back("Ext:css");

    } else if (s.compare("Lang:PERL")==0)
    {
        result.push_back("Ext:pl");
        result.push_back("Ext:pm");
        result.push_back("Ext:plx");

    } else if (s.compare("Lang:PYTHON")==0)
    {
        result.push_back("Ext:py");
        result.push_back("Ext:pyw");

    } else if (s.compare("Lang:LUA")==0)
    {
        result.push_back("Ext:lua");

    } else if (s.compare("Lang:TEX")==0)
    {
        result.push_back("Ext:tex");

    } else if (s.compare("Lang:FORTRAN")==0)
    {
        result.push_back("Ext:f");
        result.push_back("Ext:for");
        result.push_back("Ext:f90");
        result.push_back("Ext:f95");
        result.push_back("Ext:f2k");
        
    } else if (s.compare("Lang:BASH")==0)
    {
        result.push_back("Ext:sh");
        result.push_back("Ext:bsh");

    } else if (s.compare("Lang:FLASH")==0)
    {
        result.push_back("Ext:as");
        result.push_back("Ext:mx");

    } else if (s.compare("Lang:NSIS")==0)
    {
        result.push_back("Ext:nsi");
        result.push_back("Ext:nsh");
    } else if (s.compare("Lang:TCL")==0)
    {
        result.push_back("Ext:tcl");
    } else if (s.compare("Lang:LISP")==0)
    {
        result.push_back("Ext:lsp");
        result.push_back("Ext:lisp");
    } else if (s.compare("Lang:SCHEME")==0)
    {
        result.push_back("Ext:scm");
        result.push_back("Ext:md");
        result.push_back("Ext:ss");
    } else if (s.compare("Lang:ASM")==0)
    {
        result.push_back("Ext:asm");
    } else if (s.compare("Lang:DIFF")==0)
    {
        result.push_back("Ext:diff");
        result.push_back("Ext:patch");

    } else if (s.compare("Lang:PROPS")==0)
    {
        result.push_back("Ext:properties");
        
    } else if (s.compare("Lang:PS")==0)
    {
        result.push_back("Ext:ps");
    } else if (s.compare("Lang:RUBY")==0)
    {
        result.push_back("Ext:rb");
        result.push_back("Ext:rbw");
    } else if (s.compare("Lang:SMALLTALK")==0)
    {
        result.push_back("Ext:st");
    } else if (s.compare("Lang:VHDL")==0)
    {
        result.push_back("Ext:vhd");
        result.push_back("Ext:vhdl");
    } else if (s.compare("Lang:KIX")==0)
    {
        result.push_back("Ext:kix");
    } else if (s.compare("Lang:AU3")==0)
    {
        result.push_back("Ext:au3");
    } else if (s.compare("Lang:CAML")==0)
    {
        result.push_back("Ext:ml");
        result.push_back("Ext:mli");
        result.push_back("Ext:sml");
        result.push_back("Ext:thy");
           
    } else if (s.compare("Lang:ADA")==0)
    {
        result.push_back("Ext:ada");
        result.push_back("Ext:ads");
        result.push_back("Ext:adb");
          
    } else if (s.compare("Lang:VERILOG")==0)
    {
        result.push_back("Ext:v");
    } else if (s.compare("Lang:MATLAB")==0)
    {
        result.push_back("Ext:m");
    } else if (s.compare("Lang:HASKELL")==0)
    {
        result.push_back("Ext:las");
        result.push_back("Ext:as");
        result.push_back("Ext:lhs");
        result.push_back("Ext:hs");
        
    } else if (s.compare("Lang:INNO")==0)
    {
        result.push_back("Ext:iss");
    } else if (s.compare("Lang:CMAKE")==0)
    {
        result.push_back("Ext:cmake");
    } else if (s.compare("Lang:YAML")==0)
    {
        result.push_back("Ext:yml");
    } else if (s.compare("Lang:COBOL")==0)
    {
        result.push_back("Ext:cbl");
        result.push_back("Ext:cbd");
        result.push_back("Ext:cdb");
        result.push_back("Ext:cdc");
        result.push_back("Ext:cob");
            
    } else if (s.compare("Lang:GUI4CLI")==0)
    {
        result.push_back("");
    } else if (s.compare("Lang:D")==0)
    {
        result.push_back("Ext:d");
    } else if (s.compare("Lang:POWERSHELL")==0)
    {
        result.push_back("Ext:ps1");
    } else if (s.compare("Lang:R")==0)
    {
        result.push_back("Ext:r");
    } else
    {
        result.push_back("");
    }
    //alert(result);
    return result;

}

std::vector<std::string> generateScopeList()
{
    std::vector<std::string> scopeList;
    int i = 0;

    // Add Custom Scope
    char* customScopesCharArray = toCharArray(pc.configText[CUSTOM_SCOPE]);
    std::vector<std::string> customScopes = toVectorString(customScopesCharArray,'|');
    i = 0;  
    while (i<customScopes.size())  //TODO: use pop_back instead of while loop with index i
    {
        if (customScopes[i].length()!=0)
        {
            std::string customScopeString = customScopes[i];
            if (std::find(scopeList.begin(),scopeList.end(),customScopeString) == scopeList.end())
                scopeList.push_back(customScopeString);
        }
        i++;
    }
    delete [] customScopesCharArray;

    //std::string customScopeString = toString(pc.configText[CUSTOM_SCOPE]);
    //if (std::find(scopeList.begin(),scopeList.end(),customScopeString) == scopeList.end())
    //    scopeList.push_back(customScopeString);
    
    // Add the Name Scope by decomposing the current file name
    TCHAR* fileNameWide = new TCHAR[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETFILENAME, (WPARAM)MAX_PATH, (LPARAM)fileNameWide);
    char* fileName = toCharArray(fileNameWide);
    std::vector<std::string> nameParts = toVectorString(fileName,'.');
    i = 0;
    std::string namePartsString = "";
    while (i<nameParts.size())
    {
        if (nameParts[i].length()!=0)
        {
            if (i==0) 
            {
                namePartsString = "Name:" + nameParts[i];
            } else 
            {
                namePartsString = "Ext:" + nameParts[i];
            }
            if (std::find(scopeList.begin(),scopeList.end(),namePartsString) == scopeList.end())
                scopeList.push_back(namePartsString);
        }
        i++;
    }
    delete [] fileName;
    delete [] fileNameWide;

    // Add the Language Scope 
    //std::string langString = toString((char*)getLangTagType());
    std::string langString = "Lang:" + getLangTagType();
    if (std::find(scopeList.begin(),scopeList.end(),langString) == scopeList.end())
        scopeList.push_back(langString);

    // Add the Assocation Scope
    // TODO: can loop the current scopeList to add all the scope associated witht he current list, or to modify getAssociatedScope to take a vector of string directly
    std::vector<std::string> associatedScopes = getAssociatedScopes(langString);
    i = 0;
    while (i<associatedScopes.size())
    {
        if (associatedScopes[i].length()!=0)
        {
            std::string assocatedScopeString = associatedScopes[i];
            if (std::find(scopeList.begin(),scopeList.end(),assocatedScopeString) == scopeList.end())
                scopeList.push_back(assocatedScopeString);
        }
        i++;
    }
    
    // Add the NamePart Scope and ExtPart Scope for backward compatibility
    TCHAR *namePart = new TCHAR[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETNAMEPART, (WPARAM)MAX_PATH, (LPARAM)namePart);
    std::string namePartString = toString(namePart);
    if (std::find(scopeList.begin(),scopeList.end(),namePartString) == scopeList.end())
        scopeList.push_back(namePartString);
    delete [] namePart;

    TCHAR *extPart = new TCHAR[MAX_PATH];
    ::SendMessage(nppData._nppHandle, NPPM_GETEXTPART, (WPARAM)MAX_PATH, (LPARAM)extPart);
    std::string extPartString = toString(extPart);
    if (std::find(scopeList.begin(),scopeList.end(),extPartString) == scopeList.end())
        scopeList.push_back(extPartString);
    delete [] extPart;
    
    // Add the Global and System Scope        
    if (std::find(scopeList.begin(),scopeList.end(),"GLOBAL") == scopeList.end()) scopeList.push_back("GLOBAL");
    if (std::find(scopeList.begin(),scopeList.end(),"SYSTEM") == scopeList.end()) scopeList.push_back("SYSTEM");
    
        
    return scopeList;

}

bool triggerTag(int &posCurrent, int triggerLength)
{
    int paramPos = -1;
    char* previousChar;
    sciGetText(&previousChar, posCurrent-1, posCurrent);
    if (strcmp(previousChar,")")==0) paramPos = ::SendScintilla(SCI_BRACEMATCH,posCurrent-1,0);
    delete [] previousChar;
    if ((paramPos>=0) && (paramPos<posCurrent))
    {
        triggerLength = triggerLength - (posCurrent - paramPos);
        posCurrent = paramPos;
        ::SendScintilla(SCI_GOTOPOS,paramPos,0);
    }

    bool tagFound = false;
    char *tag;
	int tagLength = getCurrentTag(posCurrent, &tag, triggerLength);

    if (((triggerLength<=0) && (tag[0] == '_')) || (tagLength == 0))
    {
        delete [] tag;
    } else if (tagLength > 0) //TODO: changing this to >0 fixed the problem of tag_tab_completion, but need to investigate more about the side effect
	{
        
        int posBeforeTag = posCurrent - tagLength;

        std::vector<std::string> scopeList = generateScopeList();

        char *expanded = NULL;
        
        int i = 0;
        while ((i<scopeList.size()) && (!expanded))
        {
            expanded = findTagSQLite(tag,scopeList[i].c_str());
            i++;
        }

        // Only if a tag is found in the above process, a replace tag or trigger text completion action will be done.
        if (expanded)
        {
            if ((triggerLength<=0) && (paramPos<0))
            {
                if (pc.configInt[FALLBACK_TAB]==1) ::SendScintilla(SCI_TAB, 0, 0);
                if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
                if (pc.configInt[FALLBACK_TAB]==1) 
                {
                    ::SendScintilla(SCI_SETSELECTION, posCurrent, ::SendScintilla(SCI_GETCURRENTPOS, 0, 0));
                    ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"");
                }
            }

            if (paramPos>=0)
            {
                // Here the logic is, if it's chain snippet triggering, triggerLength>=0 and so the BEGINUNDOACTION is not going to fire. other wise, it will fire and tagFound will be equal to true.
                int paramStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                int paramEnd = ::SendScintilla(SCI_BRACEMATCH,paramStart,0) + 1;

                if (triggerLength<=0)
                {
                    ::SendScintilla(SCI_GOTOPOS,paramEnd,0);
                    if (pc.configInt[FALLBACK_TAB]==1) ::SendScintilla(SCI_TAB, 0, 0);
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
                    if (pc.configInt[FALLBACK_TAB]==1) 
                    {
                        ::SendScintilla(SCI_SETSELECTION, paramEnd, ::SendScintilla(SCI_GETCURRENTPOS, 0, 0));
                        ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"");
                    }
                    ::SendScintilla(SCI_GOTOPOS,paramStart,0);
                }

                //::SendScintilla(SCI_SETSELECTION,paramStart + 1,paramEnd - 1);
                //char* paramsContent = new char[paramEnd - 1 - (paramStart + 1) + 1];
                //::SendScintilla(SCI_GETSELTEXT,0, reinterpret_cast<LPARAM>(paramsContent));


                g_hotspotParams = smartSplit(paramStart+1,paramEnd-1,pc.configText[PARAMS_DELIMITER][0]);
                //char* paramsContent;
                //sciGetText(&paramsContent,paramStart+1,paramEnd-1);
                //
                //char paramsDelimiter = pc.configText[PARAMS_DELIMITER][0];
                //g_hotspotParams = toVectorString(paramsContent,paramsDelimiter);
                //delete [] paramsContent;
                
                //alertVector(g_hotspotParams);

                ::SendScintilla(SCI_SETSELECTION,paramStart,paramEnd);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
            }


            replaceTag(expanded, posCurrent, posBeforeTag);
            tagFound = true;
            g_lastTriggerText = tag;
        }

        //delete [] fileType;
                //if (!groupChecked) delete [] tagType;
        //delete [] tagType;
        delete [] expanded;
		delete [] tag;
        
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

void generateStroke(int vk, int modifier)
{
    //TODO: should be able to take an array of modifier
    if (modifier !=0) generateKey(modifier,true);
    generateKey(vk,true);
    generateKey(vk,false);
    if (modifier !=0) generateKey(modifier,false);
}

void generateKey(int vk, bool keyDown) 
{
    if (vk == 0) return;

    KEYBDINPUT kb = {0};
    INPUT input = {0};

    ZeroMemory(&kb, sizeof(KEYBDINPUT));
    ZeroMemory(&input, sizeof(INPUT));

    if (keyDown)
    {
        kb.dwFlags = 0;
    } else
    {
        kb.dwFlags = KEYEVENTF_KEYUP;
    }
    kb.dwFlags |= KEYEVENTF_EXTENDEDKEY;

    kb.wVk  = vk;
    input.type  = INPUT_KEYBOARD;
    input.ki  = kb;
    SendInput(1, &input, sizeof(input));
    ZeroMemory(&kb, sizeof(KEYBDINPUT));
    ZeroMemory(&input, sizeof(INPUT));
    return;
}

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
    TCHAR title[1000];
    ZeroMemory(title, sizeof(title));
    GetWindowText(hwnd, title, sizeof(title)/sizeof(title[0]));
    if(_tcsstr(title, g_tempWindowKey))
    {
        g_tempWindowHandle = hwnd;
        return false;
    }
    return true;
}

void searchWindowByName(std::string searchKey, HWND parentWindow)
{
    if (searchKey == "")
    {
        g_tempWindowHandle = nppData._nppHandle;       
    } else
    {
        char* temp = new char [searchKey.size()+1];
        strcpy(temp, searchKey.c_str());
        g_tempWindowKey = toWideChar(temp);
        
        if (parentWindow != 0)
        {
            EnumChildWindows(parentWindow, enumWindowsProc, 0);
        } else
        {
            EnumWindows(enumWindowsProc, 0);
        }
        delete [] temp;
        delete [] g_tempWindowKey;
    }
}

void setFocusToWindow()
{
    
    SetActiveWindow(g_tempWindowHandle);
    SetForegroundWindow(g_tempWindowHandle);
    
}

std::string getLangTagType()
{
    int curLang = -1;
    ::SendMessage(nppData._nppHandle,NPPM_GETCURRENTLANGTYPE ,0,(LPARAM)&curLang);
    if ((curLang>54) || (curLang<0)) return "";
    return langList[curLang];
}

void insertPrevious()
{
    //alert(g_lastTriggerText);
    char* tag = new char[g_lastTriggerText.length()+1];
    strcpy(tag,g_lastTriggerText.c_str());
    diagActivate(tag);
    delete [] tag;
}

void httpToFile(TCHAR* server, TCHAR* request, TCHAR* requestType, TCHAR* pathWide)
{
    //TODO: should change the mouse cursor to waiting
    //TODO: should report error as a return value   
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    std::vector <std::string> vFileContent;
    BOOL  bResults = false;
    HINTERNET  hSession = NULL, 
               hConnect = NULL,
               hRequest = NULL;
    
    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen( L"WinHTTP",  
                            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                            WINHTTP_NO_PROXY_NAME, 
                            WINHTTP_NO_PROXY_BYPASS, 0);

    // Specify an HTTP server.
    if (hSession)
        hConnect = WinHttpConnect( hSession, server,
                                   INTERNET_DEFAULT_HTTP_PORT, 0);
    
    // Create an HTTP request handle.
    if (hConnect)
        hRequest = WinHttpOpenRequest( hConnect, requestType, request,
                                       NULL, WINHTTP_NO_REFERER, NULL, 
                                       NULL);
    
    // Send a request.
    if (hRequest)
        bResults = WinHttpSendRequest( hRequest,
                                       WINHTTP_NO_ADDITIONAL_HEADERS,
                                       0, WINHTTP_NO_REQUEST_DATA, 0, 
                                       0, 0);

    // End the request.
    if (bResults) bResults = WinHttpReceiveResponse( hRequest, NULL);

    char* path;
    FILE * pFile;
    //TODO: move this default value part to the snippet triggering function
    if (_tcslen(pathWide) <= 0)
    {
        
        path = toCharArray(g_fttempPath);
    } else
    {
        path = toCharArray(pathWide);
    }
    pFile = fopen(path, "w+b"); 
        
    if (bResults)
    {
        do 
        {
            // Check for available data.
            dwSize = 0;
            if (!WinHttpQueryDataAvailable( hRequest, &dwSize))
            {
                showMessageBox(TEXT("Error in WinHttpQueryDataAvailable."));
            }
    
            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize+1];
    
            if (!pszOutBuffer)
            {
                showMessageBox(TEXT("Out of memory."));
                dwSize=0;
            } else
            {
                // Read the Data.
                ZeroMemory(pszOutBuffer, dwSize+1);
    
                if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
                {
                    showMessageBox(TEXT("Error in WinHttpReadData."));
                } else
                {
                    fwrite(pszOutBuffer, (size_t)dwDownloaded, (size_t)1, pFile);
                }
    
                // Free the memory allocated to the buffer.
                delete [] pszOutBuffer;
            }
        } while (dwSize>0);
    }
    fclose (pFile);

    delete [] path;

    // Report any errors.
    if (!bResults) showMessageBox(TEXT("Error has occurred."));
    // Close any open handles.
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);
}



std::vector<std::string> smartSplit(int start, int end, char delimiter, int parts)
{
    char filler;
    if (delimiter!=0)
    {
        filler = '1';
    } else 
    {
        filler = '0';
    }

    std::vector<std::string> retVal;
    std::vector<int> positions;
    char* partToSplit;
    sciGetText(&partToSplit, start, end);
    int signSpot; 
    int tailSpot;
    char* tagSignGet;
    char* tagTailGet;
    
    SendScintilla(SCI_GOTOPOS,start,0);
    do
    {
        signSpot = searchNext("\\$\\[.\\[",true);
        if ((signSpot>=start) && (signSpot < end))
        {
            
            sciGetText(&tagSignGet);
            tagTailGet = new char[4];
            strcpy(tagTailGet,"]!]");
            tagTailGet[1] = tagSignGet[2];
            SendScintilla(SCI_GOTOPOS,signSpot+1,0);
            //searchNextMatchedTail(tagSignGet,tagTailGet);
            //tailSpot = SendScintilla(SCI_GETCURRENTPOS,0,0);
            tailSpot = searchNextMatchedTail(tagSignGet,tagTailGet);
            if (tailSpot <= end && tailSpot> start)
            {
                for (int i = signSpot - start; i<tailSpot-start;i++) partToSplit[i] = filler;
            }
            delete [] tagSignGet;
            delete [] tagTailGet;
        }
    } while ((signSpot < end) && (signSpot > start));

    //alert(partToSplit);
    
    retVal = toVectorString(partToSplit,delimiter,parts);
    
    int i = 0;
    for (i = 0; i<retVal.size();i++)
    {
        positions.push_back(retVal[i].length());
    }

    int caret = start;
    char* tempString;
    for (i = 0; i<positions.size();i++)
    {
        //alertNumber(positions[i]);
        sciGetText(&tempString, caret, caret + positions[i]);
        retVal[i] = toString(tempString);
        delete [] tempString;
        caret += positions[i]+1;
    }
    delete [] partToSplit;
    return retVal;
}


void triggerDiagInsertion()
{
    if (insertionDlg.insertSnippet())
    {
        insertionDlg.display(false);
    
        insertionDlg.clearList();
        insertionDlg.clearText();
    } else
    {
        insertionDlg.completeSnippets();
    }
}

bool diagActivate(char* tag)
{
    bool retVal = false;
    //TODO: can possbily refactor and restructure with tabActivate. But need to rethink the whol structure. Better way is probably just take the simliar parts and separate it out to a function
    if (g_enable == true) 
    {
        int lineCurrent = ::SendScintilla(SCI_LINEFROMPOSITION,::SendScintilla(SCI_GETCURRENTPOS,0,0),0);

        if ((g_editorView == true) && (lineCurrent <=2))
        {
            //DO NOTHING HERE

        } else
        {
            if (::SendScintilla(SCI_AUTOCACTIVE,0,0)) ::SendScintilla(SCI_AUTOCCANCEL,0,0);
            if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
            turnOffOptionMode();

            int posSelectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
            int posSelectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);

            char* selectedText;
            sciGetText(&selectedText,posSelectionStart,posSelectionEnd);
            g_selectedText = selectedText;
            delete [] selectedText;


            SendScintilla(SCI_REPLACESEL,0,(LPARAM)tag);

            int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            int triggerLength = strlen(tag);
            bool tagFound = false;
            g_hotspotParams.clear();
            
            pc.configInt[LIVE_HINT_UPDATE]--;
            g_selectionMonitor--;
                    

            
            tagFound = triggerTag(posCurrent,triggerLength); 
            if (tagFound)
            {
                ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                g_lastTriggerPosition = posCurrent;

                
                int navSpot = 0;
                bool dynamicSpotTemp = false;
                bool dynamicSpot = false;
                bool outBound = false;
                int prevHotspotPos = -1;

                if (g_editorView == false)
                {
                    int i;
                    if (!tagFound) //TODO: as tagFound is checked to be true 10 lines ago.....there is something wrong here?
                    {

                        i = g_listLength-1;
                        if (posCurrent > g_lastTriggerPosition)
                        {
                            do
                            {
                                //TODO: limit the search to g_lastTriggerPosition ?       
                                prevHotspotPos = searchPrev(g_tagSignList[i]);

                                

                                if (prevHotspotPos >= 0)
                                {
                                    if (prevHotspotPos < g_lastTriggerPosition)
                                    {
                                        outBound = true;
                                    }
                                    
                                    ::SendScintilla(SCI_GOTOPOS,g_lastTriggerPosition,0);
                                    posCurrent = g_lastTriggerPosition;
                    
                                    //posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                                    //posCurrent = 0;
                                    //g_lastTriggerPosition = posCurrent;
                                    break;   
                                }
                                i--;
                            } while (i>=0);

                        }

                    }

                    i = g_listLength - 1;
                    
                    do
                    {
                        if (dynamicSpot)
                        {
                            dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                        } else
                        {
                            dynamicSpot = dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                        }
                        ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
                        
                        navSpot = hotSpotNavigation(g_tagSignList[i],g_tagTailList[i]);
                        
                        i--;
                    } while ((navSpot <= 0) && (i >= 0));

                    if ((!((navSpot > 0) || (dynamicSpot))) && (outBound))
                    {
                        int prevPos = g_lastTriggerPosition;
                        do
                        {
                            g_lastTriggerPosition = prevPos;
                            prevPos = searchPrev("\\$\\[.\\[",true);
                        } while (prevPos>=0);

                        //::SendScintilla(SCI_GOTOPOS,0,0);
                        //g_lastTriggerPosition = 0;
                        posCurrent = g_lastTriggerPosition;

                        i = g_listLength - 1;
                        do
                        {
                            if (dynamicSpot)
                            {
                                dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                            } else
                            {
                                dynamicSpot = dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                            }
                            ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
                            
                            navSpot = hotSpotNavigation(g_tagSignList[i],g_tagTailList[i]);
                            
                            i--;
                        } while ((navSpot <= 0) && (i >= 0));
                    }

                    

                    //TODO: this line is position here so the priority spot can be implement, but this cause the 
                    //      1st hotspot not undoable when the snippet is triggered. More investigation on how to
                    //      manipulate the undo list is required to make these 2 features compatible
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);

                    if (navSpot != 3)
                    {
                        if ((navSpot > 0) || (dynamicSpot)) ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                    }

                    if ((navSpot > 0) || (dynamicSpot)) g_onHotSpot = true;
                    
                } else
                {
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
                }

                retVal = true;
                
            } else
            {
                
                // clear the tag
                ::SendScintilla(SCI_SETSEL,posCurrent-triggerLength, posCurrent);
                ::SendScintilla(SCI_REPLACESEL,0,(LPARAM)"");
                retVal = false;
                if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
            }
            
            g_selectionMonitor++;
            pc.configInt[LIVE_HINT_UPDATE]++;

        }
     }

     
     return retVal;

}

void tabActivate()
{
    doTabActivate();
}

void doTabActivate(bool navOnly)
{
    if (sciFocus)
    {
        //if (((g_enable==false) || (g_rectSelection==true) || (::SendScintilla(SCI_AUTOCACTIVE,0,0))) && (!g_optionMode))
        if (((g_enable==false) || (g_rectSelection==true) ) && (!g_optionMode) && (!g_onHotSpot))
        {        
            ::SendScintilla(SCI_TAB,0,0);   
        } else
        {
            int autoComplete = 0;

            if (::SendScintilla(SCI_AUTOCACTIVE,0,0))
            {
                ::SendScintilla(SCI_AUTOCCOMPLETE,0,0);
                autoComplete = 1;
            }
            int posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
            
            //int posTriggerStart = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
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
                bool tagFound = false;
                g_hotspotParams.clear();
                
                pc.configInt[LIVE_HINT_UPDATE]--;
                g_selectionMonitor--;
                
                int posSelectionStart = ::SendScintilla(SCI_GETSELECTIONSTART,0,0);
                int posSelectionEnd = ::SendScintilla(SCI_GETSELECTIONEND,0,0);

                if (g_optionMode)
                {
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
                    //turnOffOptionMode();
                    //::SendScintilla(SCI_GOTOPOS,g_optionEndPosition,0);
                    //snippetHintUpdate();
                } else
                {             

                    if ((posSelectionStart==posSelectionEnd) && (!navOnly)) tagFound = triggerTag(posCurrent); 
                    if (tagFound)
                    {
                        //::SendScintilla(SCI_AUTOCCANCEL,0,0);
                        g_selectedText = "";
                        posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                        g_lastTriggerPosition = posCurrent;
                        
                    } else
                    {

                        if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
                    }
                }
                // A SCI_BEGINUNDOACTION message is sent in triggerTag. So after this point no matter in which case
                // the SCI_BEGINUNDOACTION is sent.


                int navSpot = 0;
                bool dynamicSpotTemp = false;
                bool dynamicSpot = false;
                bool outBound = false;
                int prevHotspotPos = -1;

                if (g_editorView == false)
                {
                    int i;
                    if (!tagFound) 
                    {
                        i = g_listLength-1;
                        if (posCurrent > g_lastTriggerPosition)
                        {
                            do
                            {
                                //TODO: limit the search to g_lastTriggerPosition ?       
                                prevHotspotPos = searchPrev(g_tagSignList[i]);

                                

                                if (prevHotspotPos >= 0)
                                {
                                    if (prevHotspotPos < g_lastTriggerPosition)
                                    {
                                        outBound = true;
                                    }
                                    
                                    ::SendScintilla(SCI_GOTOPOS,g_lastTriggerPosition,0);
                                    posCurrent = g_lastTriggerPosition;
                    
                                    //posCurrent = ::SendScintilla(SCI_GETCURRENTPOS,0,0);
                                    //posCurrent = 0;
                                    //g_lastTriggerPosition = posCurrent;
                                    break;   
                                }
                                i--;
                            } while (i>=0);
                        }
                    
                    }
                    //TODO: Params inertion will stop when navSpot is true, so it is not working properly under differnt level of priority
                    //      Or in other words it only work for the highest existing level of priority
                    
                    
                    i = g_listLength - 1;
                    do
                    {
                        if (dynamicSpot)
                        {
                            dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                        } else
                        {
                            dynamicSpot = dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                        }
                        ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
                        
                        navSpot = hotSpotNavigation(g_tagSignList[i],g_tagTailList[i]);
                        
                        i--;
                    } while ((navSpot <= 0) && (i >= 0));
                    
                    if ((!((navSpot > 0) || (dynamicSpot))) && (outBound))
                    {
                        int prevPos = g_lastTriggerPosition; //TODO: should be able to restructure and use the prevHotspotPos defined before
                        do
                        {
                            g_lastTriggerPosition = prevPos;
                            prevPos = searchPrev("\\$\\[.\\[",true);
                        } while (prevPos>=0);

                        //::SendScintilla(SCI_GOTOPOS,0,0);
                        //g_lastTriggerPosition = 0;
                        posCurrent = g_lastTriggerPosition;

                        i = g_listLength - 1;
                        do
                        {
                            if (dynamicSpot)
                            {
                                dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                            } else
                            {
                                dynamicSpot = dynamicHotspot(posCurrent,g_tagSignList[i],g_tagTailList[i]);
                            }
                            ::SendScintilla(SCI_GOTOPOS,posCurrent,0);
                            
                            navSpot = hotSpotNavigation(g_tagSignList[i],g_tagTailList[i]);
                            
                            i--;
                        } while ((navSpot <= 0) && (i >= 0));
                    }
                    
                    
                    //TODO: this line is position here so the priority spot can be implement, but this cause the 
                    //      1st hotspot not undoable when the snippet is triggered. More investigation on how to
                    //      manipulate the undo list is required to make these 2 features compatible
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);

                    if (navSpot != 3)
                    {
                        if ((navSpot > 0) || (dynamicSpot)) ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                    }

                    if ((navSpot > 0) || (dynamicSpot)) g_onHotSpot = true;
                } else
                {
                    if (pc.configInt[PRESERVE_STEPS]==0) ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
                }

                bool snippetHint = false;

                if ((g_onHotSpot) && (posSelectionStart!=posSelectionEnd))
                {
                    if ((navSpot == 0) && (tagFound == false) && (dynamicSpot==false)) 
                    {
                        ::SendScintilla(SCI_GOTOPOS, posSelectionEnd, 0);
                        g_onHotSpot = false;
                    }

                } else
                {
                    

                    int completeFound = -1;
                    if (pc.configInt[TAB_TAG_COMPLETION] == 1)
                    {
                        if ((navSpot == 0) && (tagFound == false) && (dynamicSpot==false)) 
	        	        {
                            
                            ::SendScintilla(SCI_GOTOPOS, posSelectionStart, 0);
                            posCurrent = posSelectionStart;

                            if (pc.configInt[FALLBACK_TAB]==1) ::SendScintilla(SCI_TAB, 0, 0);
                            ::SendScintilla(SCI_BEGINUNDOACTION, 0, 0);
                            if (pc.configInt[FALLBACK_TAB]==1)
                            {
                                ::SendScintilla(SCI_SETSELECTION, posSelectionStart, ::SendScintilla(SCI_GETCURRENTPOS, 0, 0));
                                ::SendScintilla(SCI_REPLACESEL, 0, (LPARAM)"");
                            }
                            //completeFound = snippetComplete();
                            //completeFound = triggerTag(posCurrent,true);
                            completeFound = tagComplete();
                            ::SendScintilla(SCI_ENDUNDOACTION, 0, 0);
                            if (completeFound>=0)
                            {
                                ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                                snippetHint = true;
                            }
	        	        }
                    }
                    
                    if ((navSpot == 0) && (tagFound == false) && (completeFound<0) && (dynamicSpot==false) && (autoComplete==0)) 
                    {
                        if (g_optionMode == true)
                        {
                            turnOffOptionMode();
                            ::SendScintilla(SCI_GOTOPOS,g_optionEndPosition,0);
                            snippetHint = true;
                        } else
                        {
                            //g_tempWindowHandle = (HWND)::SendMessage(nppData._nppHandle,NPPM_DMMGETPLUGINHWNDBYNAME ,(WPARAM)TEXT("SherloXplorer"),(LPARAM)TEXT("SherloXplorer.dll"));
                            //setFocusToWindow();
                            //generateKey(toVk("TAB"),true);
                            //generateKey(toVk("TAB"),false);
                            restoreTab(posCurrent, posSelectionStart, posSelectionEnd);
                        }
                    }
                }
                pc.configInt[LIVE_HINT_UPDATE]++;
                if (snippetHint) snippetHintUpdate();
                g_selectionMonitor++;
                //g_onHotSpot = true;
            }
            
        }
    }
}



LRESULT CALLBACK KeyboardProc(int ncode,WPARAM wparam,LPARAM lparam)
{
	if(ncode==0)
	{
		if((lparam & 0x80000000) == 0x00000000)
		{
            if ((sciFocus == 1) && (wparam == VK_TAB))
            {
                if( !(GetKeyState(VK_SHIFT)&0x8000) && !(GetKeyState(VK_CONTROL)&0x8000) && !(GetKeyState(VK_MENU)&0x8000) )
                {
                    tabActivate(); 
                }
            }
            
		}
	}
	return ( CallNextHookEx(hook,ncode,wparam,lparam) );//pass control to next hook in the hook chain.
}


void installhook()
{
	hook = NULL;
	hook = SetWindowsHookEx(WH_KEYBOARD,KeyboardProc,(HINSTANCE)g_hModule,::GetCurrentThreadId());
}

void removehook()
{
	UnhookWindowsHookEx(hook);
}

//void testThread( void* pParams )
//{ 
//    //system("npp -multiInst");
//    for (int i = 0; i<100;i++)
//    {
//        SendScintilla(SCI_REPLACESEL,0,(LPARAM)toCharArray(toString((double)sciFocus)));
//        Sleep(1000);
//    }
//    
//}

void testing2()
{
    alert("testing2");
    
    alert(g_snippetCount);
    
    //::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
    //LRESULT importEditorBufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
    //::SendMessage(nppData._nppHandle, NPPM_SETBUFFERENCODING, (WPARAM)importEditorBufferID, 4);



}

void testing()
{
    alert("testing1");
       

    ////Testing getpixel
    //HDC hdc = GetDC(0);
    //COLORREF color = ::GetPixel(hdc,100,100);
    ////COLORREF color = RGB(255,128,64);
    //alert((int)GetRValue(color));
    //alert((int)GetGValue(color));
    //alert((int)GetBValue(color));



    ////Testing drawing pixel on screen
    //COLORREF color = RGB(255,0,0); // COLORREF to hold the color info
	//HDC hdc = GetDC(0); // Get the DC from that HWND
    //int x = 0;
    //int y = 0;
    //
    //for (x = 0; x<200; x++)
    //{
    //    for (y = 0; y<200; y++)
    //    {
    //        if (((x%2) && (!(y%2))) || ((y%2) && (!(x%2))))
    //        {
	//            SetPixel(hdc, x, y, color); // SetPixel(HDC hdc, int x, int y, COLORREF color)
    //         
    //        }
    //
    //    }
    //}
    //ReleaseDC(0, hdc); // Release the DC
	//DeleteDC(hdc); // Delete the DC


    ////Testing drawing on screen
    //HDC screenDC = ::GetDC(0);
    //::Rectangle(screenDC, 200, 200, 300, 300);
    //::ReleaseDC(0, screenDC);
    //DeleteDC(screenDC);


    ////Testing drawing on screen
    //HDC screenDC = ::GetDC(nppData._scintillaMainHandle);
    //::Rectangle(screenDC, 200, 200, 300, 300);
    //::ReleaseDC(nppData._scintillaMainHandle, screenDC);
    //DeleteDC(screenDC);

    //// Search of items in listbox
    //alert(snippetDock.searchSnippetList(TEXT("<GLOBAL>      div")));


     ////Test case for snippetTextBrokenDown
     //std::vector<std::string> vs;
     //vs.push_back("awesome");
     //vs.push_back("hello)");
     //TCHAR* result;
     //char* triggerText = new char[100];
     //strcpy(triggerText,"awesome");
     //char* content = new char[200];
     //strcpy(content,"<div id=\"$[![myid]!]\" class=\"$[![myclass]!]\">  $[![(key)$[![keyword]!]]!] $[1[priority]1] This id is $[![myid]!] and the class is $[![myclass]!]  $[![]!]</div>");
     //result = snippetTextBrokenDown(vs,triggerText,content);
     //alert(result);
     //delete [] result;


    //// create new scintillahandle
    //HWND g_customSciHandle;
    //g_customSciHandle = (HWND)::SendMessage(nppData._nppHandle,NPPM_CREATESCINTILLAHANDLE,0,NULL);
    //alert(::SendMessage(g_customSciHandle,SCI_GETCURRENTPOS,0,0));
    //::SendMessage(g_customSciHandle,SCI_REPLACESEL,0,(LPARAM)"12345");
    //::SendMessage(g_customSciHandle,SCI_GOTOPOS,3,3);
    //alert(::SendMessage(g_customSciHandle,SCI_GETCURRENTPOS,0,0));
    //::SendMessage(nppData._nppHandle,NPPM_DESTROYSCINTILLAHANDLE,0,(LPARAM)&g_customSciHandle);
    //alert("Hello");


    //// Testing replaceall
    //char* ori = new char[100];
    //char* key = new char[10];
    //char* rep = new char[10];
    //strcpy(ori,"Hello Hello everybody!");
    //strcpy(key,"Hello");
    //strcpy(rep,"wowowowow");
    //
    //alert((int)strlen(ori));
    //ori = replaceAll(ori,key,rep);
    //alert(ori);
    //alert((int)strlen(ori));
    //
    //delete [] ori;
    //delete [] key;
    //delete [] rep;



    //
    //// Disable tab key in scintilla
    //SendScintilla(SCI_ASSIGNCMDKEY,SCK_TAB,SCI_NULL);



    //// Testing thread
    //_beginthread( testThread, 0, NULL );

    //// Testing sorting
    //std::vector<std::string> test = toVectorString("BAR|DOOR|CAT_1|KING|CAT|CUP",'|');
    //alert(test);
    //alert(toSortedVectorString(test));
    //

    //// Testing alert()
    //alert();
    //alert(5);
    //alert(0.375);
    //alert('I');
    //alert("Hello");
    //alert(TEXT("World"));
    //std::string testing = "FOO";
    //alert(testing);
    //std::vector<std::string> testing2 = toVectorString("BAR|DOOR|CAT_1|KING|CAT|CUP",'|');
    //alert(testing2);
    //std::vector<int> testing3;
    //testing3.push_back(3);
    //testing3.push_back(2);
    //testing3.push_back(1);
    //alert(testing3);
    //alert(nppData._nppHandle);
    //
    //

    //// Testing disable item
    // HMENU hMenu = (HMENU)::SendMessage(nppData._nppHandle, NPPM_GETMENUHANDLE, 0, 0);
    //::EnableMenuItem(hMenu, funcItem[0]._cmdID, MF_BYCOMMAND | (false?0:MF_GRAYED));
    //::ModifyMenu(hMenu, funcItem[1]._cmdID, MF_BYCOMMAND | MF_SEPARATOR, 0, 0);

    ////Manipulate windows handle
    //char buffer [100];
    //sprintf(buffer, "0x%08x", (unsigned __int64) g_tempWindowHandle);
    //alertCharArray(buffer);

    //long handle = reinterpret_cast<long>(g_tempWindowHandle);
    //alertNumber(handle);
    //HWND newWin = reinterpret_cast<HWND>(handle);


    ////Test regexp
    //alertNumber(searchNext("\\$\\[.\\[",true));


    

    ////Testing SchintillaGetText
    //char* temp;
    //sciGetText(&temp,10,20);
    //alertCharArray(temp);
    //delete [] temp;
    //char* temp2;
    //sciGetText(&temp2,13,13);
    //alertCharArray(temp2);
    //delete [] temp2;


    ////Test calltip
    //::SendScintilla(SCI_CALLTIPSHOW,0,(LPARAM)"Hello World!");


    //// test getting windows by enum windows
    //searchWindowByName("RGui");
    //searchWindowByName("R Console",g_tempWindowHandle);
    //setFocusToWindow();
    //::generateKey(toVk("CONTROL"), true);
    //::generateStroke(toVk("V"));
    //::generateKey(toVk("CONTROL"), false);
    //::generateStroke(VK_RETURN);
    
    //HWND tempWindowHandle = ::FindWindowEx(g_tempWindowHandle, 0, TEXT("Edit"),0);

     

    ////testing generatekey and get other windows
    //HWND hwnd;
    //hwnd = FindWindow(NULL,TEXT("RGui"));
    //
    //
    //SetActiveWindow(hwnd);
    //SetForegroundWindow(hwnd);
    //
    //::GenerateKey(VK_CONTROL, true);
    //::GenerateKey(0x56, true);
    //::GenerateKey(0x56, false);
    //::GenerateKey(VK_CONTROL, false);
    //
    ////::Sleep(1000);
    //::GenerateKey(VK_RETURN, true);
    //::GenerateKey(VK_RETURN, false);
    //::Sleep(1000);
    //
    //SetActiveWindow(nppData._nppHandle);
    //SetForegroundWindow(nppData._nppHandle);


    ////Testing Find and replace
    //std::string str1 = "abcdecdf";
    //std::string str2 = "cd";
    //std::string str3 = "ghicdjk";
    //alertString(str1);
    //alertString(str2);
    //alertString(str3);
    //
    //findAndReplace(str1,str2,str3);
    //alertString(str1);

    ////Testing vector<string>
    //int i;
    //char* teststr = new char[100];
    //strcpy(teststr, "abc def  ghi");
    //alertCharArray(teststr);
    //
    //std::vector<std::string> v;
    //    
    //v = toVectorString(teststr,' ');
    //delete [] teststr;
    //i = 0;
    //while (i<v.size())
    //{
    //    alertString(v[i]);
    //    i++;
    //}


    ////Testing brace match
    //int result = ::SendScintilla(SCI_BRACEMATCH,3,0);
    //alertNumber(result);


    //// For opening static dialog
    //openDummyStaticDlg();

    
    //Testing usage of quickStrip()
    //char* test = new char[MAX_PATH];
    //strcpy(test,"To test the quickStrip.");
    //alertCharArray(test);
    //test = quickStrip(test,'t');
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


    //alertNumber(pc.snippetListLength);
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

    

    //pc.configText[CUSTOM_SCOPE] = TEXT(".cpp");
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











}
