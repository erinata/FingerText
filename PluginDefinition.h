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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H


#include "PluginInterface.h"

#include <string>       // For alertString()
#include <vector>
#include <tchar.h>      // For _tcscpy, _tcscat 
#include <fstream>      // For file reading and writing
#include <winhttp.h>    // For http requests, Add winhttp.lib to additional dependencies if there is external definition error
#include <process.h>    // For thread
#include <regex>

#include "PluginConfig.h"
#include "sqlite3.h"
#include "InsertionDialog.h"
#include "SnippetDock.h"
#include "Version.h"
#include "DuckEval.h"
#include "NppApiHelpers.h"
#include "StringUtils.h"
#include "ConversionUtils.h"
#include "DebugUtils.h"

// Functions that are loaded when notepad++ starts
void pluginInit(HANDLE hModule);   // Initialization of your plugin data
void commandMenuInit();            // Initialization of your plugin commands, setCommand is a function that helps initialization
void dialogsInit();                // Initialization of dialogs instance
void pathInit();                   // Setup paths (everything other than ini and database paths
void configInit();                 // Setup config ini file
void dataBaseInit();               // Setup Sqlite database
void variablesInit();              // Initialization of other variables
void nppReady();                   // Called when NPP_READY is fired

// Functions that are loaded when notepad++ shutdown
void commandMenuCleanUp();         //Clean up plugin commands allocation
void pluginCleanUp();              // Cleaning of your plugin; called while plugin unloading
void pluginShutdown();             // Called when Notepad++ shuts down

// common Functions for a plugin
void showSettings();
void showHelp();
void showAbout();

// Custom functions for Fingertext
void shortCutRemapped();
char *findTagSQLite(char *tag, const char *tagCompare);
void showInsertionDlg();
void setInsertionDialogState(int state);

void toggleDisable();
void restoreTab(int &posCurrent, int &posSelectionStart, int &posSelectionEnd);
int searchPrevMatchedSign(char* tagSign, char* tagTail);
int searchPrevMatchedTail(char* tagSign, char* tagTail);
bool dynamicHotspot(int &startingPos, char* tagSign = "$[![", char* tagTail = "]!]");
void paramsInsertion(int &firstPos, char* hotSpot, int &checkPoint);
void textCopyCut(int sourceType, int operationType, int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint);
void keyWordSpot(int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint);
void webRequest(int &firstPos, char* hotSpotText);
void executeCommand(int &firstPos, char* hotSpotText);
void launchMessageBox(int &firstPos, char* hotSpotText);
std::string evaluateCall(char* expression);
void evaluateHotSpot(int &firstPos, char* hotSpotText);
void chainSnippet(int &firstPos, char* hotSpotText);
int hotSpotNavigation(char* tagSign = "$[![", char* tagTail = "]!]");

int grabHotSpotContent(char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength, int tailPos);
void showPreview(bool top = false,bool insertion = false);

void doSelectionToSnippet();
void selectionToSnippet(bool forceNew = false);
void insertHotSpotSign();
//void insertWarmSpotSign();
//void goToWarmSpot();
//bool warmSpotNavigation(HWND &curScintilla);
//void insertChainSnippetSign();
//void insertKeyWordSpotSign();
//void insertCommandLineSign();
void searchAndReplace(std::string key, std::string text, bool regExp = false);
void insertTagSign(char * tagSign);
bool getLineChecked(char **buffer, int lineNumber, TCHAR* errorText);
bool replaceTag(char *expanded, int &posCurrent, int &posBeforeTag);
void openDatabase();
int getCurrentTag(int posCurrent, char **buffer, int triggerLength = 0);

void showSnippetDock();
void updateDockItems(bool withContent = true, bool withAll = false, char* tag = "%", bool populate = false, bool populateInsertion = false, bool searchType = false);
void populateDockItems(bool withAll = true, bool insertion = false);

void setTextTarget(bool fromTab);
void setListTarget();
void saveSnippet();
const char* getLangTagType();


void updateInsertionDialogHint();
//void deleteCache();
//void clearCache();
void insertSnippet();
void closeEditor();
void editSnippet();
void deleteSnippet();
void exportSnippetsOnly();
void clearAllSnippets();
void exportAndClearSnippets();
bool exportSnippets(bool all = true);
void importSnippets();
bool snippetHintUpdate();
int promptSaveSnippet(TCHAR* message = NULL);

void httpToFile(TCHAR* server, TCHAR* request, TCHAR* requestType, TCHAR* path = TEXT(""));

void updateMode();
void refreshAnnotation();
void updateLineCount(int count = -1);
char* getDateTime(char *format, bool getDate = true, int flags = 0);

//void insertDateTime(bool date,int type, HWND &curScintilla);
void insertPath(TCHAR* path);
void insertNppPath(int msg);

void writeInsertMode(bool insert);

void generateStroke(int vk, int modifier = 0);
void generateKey(int vk, bool keyDown);
BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam);
void setFocusToWindow();
void searchWindowByName(std::string searchKey = "", HWND parentWindow = 0);

//std::vector<std::string> snippetTextBrokenDown(std::string editText, std::vector<std::string> vs, char* tempTriggerText, char* snippetContent, int position);
std::vector<std::string> snippetTextBrokenDown(std::string editText, std::vector<std::string> params, char** tempTriggerText, char** snippetContent, int position);

void selectionMonitor(int contentChange);
bool triggerTag(int &posCurrent,int triggerLength = 0);
int tagComplete();
void doTagComplete();
bool diagActivate(char* tag);
void triggerDiagInsertion();

void tabActivate();

std::vector<std::string> smartSplit(int start, int end, char delimiter, int parts = 0);

void removehook();
void installhook();



void closeNonSessionTabs();

void updateOptionCurrent(bool toNext);
void cleanOptionItem();
void turnOffOptionMode();
void turnOnOptionMode();
void optionNavigate(bool toNext);

void testing();
void testing2();


#endif //PLUGINDEFINITION_H