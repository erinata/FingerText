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
#include <tchar.h>      // For TEXT() and _tcscpy, _tcscat 
#include <fstream>      // For file reading and writing
#include <winhttp.h>    // For http requests, Add winhttp.lib to additional dependencies if there is external definition error
#include <process.h>    // For thread

#include "sqlite3.h"
#include "DummyStaticDialog.h"
#include "SnippetDock.h"
#include "Version.h"
#include "Evaluate.h"

// 
const TCHAR NPP_PLUGIN_NAME[] = TEXT(PLUGIN_NAME);   // Plugin name
const int nbFunc = MENU_LENGTH;    // Number of your plugin commands

// Functions in the plugin template(NppPluginTemplate) provided by Don Ho. 
void pluginInit(HANDLE hModule);   // Initialization of your plugin data; called while plugin loading
void pluginCleanUp();    // Cleaning of your plugin; called while plugin unloading
void commandMenuInit();   //Initialization of your plugin commands, setCommand is a function that helps initialization
void dialogsInit();
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);
void commandMenuCleanUp();  //Clean up plugin commands allocation
void pluginShutdown();  // Called when Notepad++ shuts down

// Connecting to Scintilla
HWND getCurrentScintilla();
sptr_t SendScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
void updateScintilla();

// Functions for Fingertext
void openDummyStaticDlg(void);
void toggleDisable();
void writeConfig();
void saveCustomScope();
void writeConfigTextChar(TCHAR* configChar, TCHAR* section);
void resetDefaultSettings();
void writeConfigText(int configInt, TCHAR* section);
void restoreTab(int &posCurrent, int &posSelectionStart, int &posSelectionEnd);
int searchPrevMatchedSign(char* tagSign, char* tagTail);
int searchPrevMatchedTail(char* tagSign, char* tagTail);
bool dynamicHotspot(int &startingPos);
void paramsInsertion(int &firstPos, char* hotSpot, int &checkPoint);
void keyWordSpot(int &firstPos, char* hotSpotText, int &startingPos, int &checkPoint);
void executeCommand(int &firstPos, char* hotSpotText);
void launchMessageBox(int &firstPos, char* hotSpotText);
void evaluateExpression(int &firstPos, char* hotSpotText);
void chainSnippet(int &firstPos, char* hotSpotText);
bool hotSpotNavigation();
int grabHotSpotContent(char **hotSpotText,char **hotSpot, int firstPos, int &secondPos, int signLength);
void showPreview(bool top = false);
void emptyFile(TCHAR* fileName);
void writeDefaultGroupFile();
int searchNext(char* searchText);
int searchPrev(char* searchText);
void selectionToSnippet();
void insertHotSpotSign();
//void insertWarmSpotSign();
//void goToWarmSpot();
//bool warmSpotNavigation(HWND &curScintilla);
bool hotSpotNavigation(HWND &curScintilla);
//void insertChainSnippetSign();
//void insertKeyWordSpotSign();
//void insertCommandLineSign();
bool getLineChecked(char **buffer, int lineNumber, TCHAR* errorText);
void insertTagSign(char * tagSign);
bool replaceTag(char *expanded, int &posCurrent, int &posBeforeTag);
void openDatabase();
int getCurrentTag(int posCurrent, char **buffer, int triggerLength = 0);
void pluginShutdown();

void setupConfigFile();
void convertToUTF8(TCHAR *orig, char **utf8);
void showSnippetDock();
void updateDockItems(bool withContent = false, bool withAll = false, char* tag = "%");
void populateDockItems();
void initialize();
void upgradeMessage();
void saveSnippet();
//void createSnippet();
char* getLangTagType();
char* cleanupString( char *str );
void clearCache();
void editSnippet();
void deleteSnippet();
void insertSnippet();
void exportSnippetsOnly();
void clearAllSnippets();
void exportAndClearSnippets();
bool exportSnippets();
void importSnippets();
void snippetHintUpdate();
int promptSaveSnippet(TCHAR* message = NULL);

void updateMode();
void refreshAnnotation();
void settings();
void showHelp();
void showAbout();
void updateLineCount(int count = -1);
char* getDateTime(char *format, bool getDate = true, int flags = 0);
void convertToWideChar(char* orig, wchar_t **wideChar);
//void insertDateTime(bool date,int type, HWND &curScintilla);
void insertPath(TCHAR* path);
void insertNppPath(int msg);

void selectionMonitor(int contentChange);
bool triggerTag(int &posCurrent,bool triggerTextComplete = false,int triggerLength = 0);
void tagComplete();
//bool snippetComplete();
void tabActivate();

void updateOptionCurrent(bool toNext);
void addOptionItem(char* item);
void cleanOptionItem();
void turnOffOptionMode();
void optionNavigate(bool toNext);

void testing();
void testing2();
void alert();
void alertNumber(int input);
void alertCharArray(char* input);
void alertTCharArray(TCHAR* input);
void alertString(std::string input);
void alertVector(std::vector<std::string> v);
void findAndReplace(std::string& str, const std::string& oldStr, const std::string& newStr);

std::vector<std::string> split(char* str, char c = ' ');
//std::vector<std::string> split2(char* str, char c1, char c2, char c3);

#endif //PLUGINDEFINITION_H

