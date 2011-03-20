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

#ifndef PLUGINDEFINITION_H
#define PLUGINDEFINITION_H

//
// All difinitions of plugin interface
//


#include "PluginInterface.h"
#include <fstream>

//-------------------------------------//
//-- STEP 1. DEFINE YOUR PLUGIN NAME --//
//-------------------------------------//
// Here define your plugin name
//
const TCHAR NPP_PLUGIN_NAME[] = TEXT("FingerText");
//-----------------------------------------------//
//-- STEP 2. DEFINE YOUR PLUGIN COMMAND NUMBER --//
//-----------------------------------------------//
//
// Here define the number of your plugin commands
//
const int nbFunc = 18;


//
// Initialization of your plugin data
// It will be called while plugin loading
//
void pluginInit(HANDLE hModule);

//
// Cleaning of your plugin
// It will be called while plugin unloading
//
void pluginCleanUp();

//
//Initialization of your plugin commands
//
void commandMenuInit();

//
//Clean up your plugin commands allocation (if any)
//
void commandMenuCleanUp();

//
// Function which sets your command 
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk = NULL, bool check0nInit = false);

// Called when Notepad++ shuts down
void pluginShutdown();
//
// Your plugin command functions
//

HWND getCurrentScintilla();
void toggleDisable();
void writeConfig();
void resetDefaultSettings();
void writeConfigText(int configInt, TCHAR* section);
void restoreTab(HWND &curScintilla, int &posCurrent, int &posSelectionStart, int &posSelectionEnd);
void dynamicHotspot(HWND &curScintilla, int &startingPos, int hotSpotType);
void keyWordSpot(HWND &curScintilla, int &firstPos, char* hotSpotText, int &startingPos);
void executeCommand(HWND &curScintilla, int &firstPos, char* hotSpotText);
void chainSnippet(HWND &curScintilla, int &firstPos, char* hotSpotText);
bool hotSpotNavigation(HWND &curScintilla);
int grabHotSpotContent(HWND &curScintilla, char **hotSpotText,char **hotSpot, int firstPos, int signLength);
void showPreview();
void emptyFile(TCHAR* fileName);
int searchNext(HWND &curScintilla, char* searchText);
int searchPrev(HWND &curScintilla, char* searchText);
void selectionToSnippet();
void insertHotSpotSign();
void insertWarmSpotSign();
void goToWarmSpot();
bool warmSpotNavigation(HWND &curScintilla);
bool hotSpotNavigation(HWND &curScintilla);
//void insertChainSnippetSign();
//void insertKeyWordSpotSign();
//void insertCommandLineSign();
bool getLineChecked(char **buffer, HWND &curScintilla, int lineNumber, TCHAR* errorText);
void insertTagSign(char * tagSign);
bool getLineChecked(char **buffer, HWND &curScintilla, int lineNumber, TCHAR* errorText, bool import);
bool replaceTag(HWND &curScintilla, char *expanded, int &posCurrent, int &posBeforeTag);
void openDatabase();
int getCurrentTag(HWND curScintilla, int posCurrent, char **buffer, int triggerLength=0);
void pluginShutdown();
void setConfigAndDatabase();
void setupConfigFile();
void convertToUTF8(TCHAR *orig, char **utf8);
void showSnippetDock();
void updateDockItems(bool withContent=false, bool withAll=false, char* tag = "%");
void populateDockItems();
void initialize();
void upgradeMessage();
void saveSnippet();
//void createSnippet();
void clearCache();
void editSnippet();
void deleteSnippet();
void insertSnippet();
void exportSnippets();
void importSnippets();
void snippetHintUpdate();
int promptSaveSnippet(TCHAR* message = NULL);
void updateMode();
void refreshAnnotation();
void settings();
void showHelp();
void showAbout();
void insertDateTime(bool date,int type, HWND &curScintilla);
void insertPath(TCHAR* path, HWND &curScintilla);
void insertNppPath(int msg, HWND &curScintilla);
void executeCommand();
bool triggerTag(int &posCurrent, int triggerLength=0);
void tagComplete();
bool snippetComplete();
void fingerText();

void testing();
void alert();

#endif //PLUGINDEFINITION_H
