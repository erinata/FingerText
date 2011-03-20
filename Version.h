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

//TODO: simplify version.h
#define VERSION_TEXT "0.5.6.0"
#define VERSION_NUM 0,5,6,0
#define VERSION_LINEAR 506
#define VERSION_KEEP_CONFIG_START 500
#define VERSION_KEEP_CONFIG_END 506
#define VERSION_TEXT_LONG "FingerText 0.5.6.0(Alpha)"
#define VERSION_TEXT_FULL TEXT("\
FingerText 0.5.6.0(Alpha)\r\n\
February 2011\r\n\r\n\
Author: Tom Lam\r\n\
Email: erinata@gmail.com\r\n\r\n\
Update to the lastest version:\r\n\
     http://sourceforge.net/projects/fingertext/ \r\n\
Usage Guide and Source code:\r\n\
     http://github.com/erinata/FingerText \r\n\r\n\
(Snippets created using FingerText version earlier than 0.3.5 are not compatible with this version)\
")

#define HELP_TEXT_FULL TEXT("\
FingerText Quick Guide:\r\n\r\n\
Insert Snippet --- Type in TriggerText and Hit the tab key\r\n\
Navigate to next Hotspot --- Hit the tab key\r\n\
Show SnippetDock --- Menu>Plugins>FingerText>Toggle On/Off SnippetDock\r\n\
AutoComplete TriggerText --- Menu>Plugins>FingerText>TriggerText Completion\r\n\
Goto Snippet Editor --- Double click snippets on the SnippetDock\r\n\
Create New Snippet --- Click Create button on the SnippetDock\r\n\
Create snippet from selection -- Menu>Plugins>FingerText>Create snippet from selection\r\n\
Save Snippet --- In the Snippet Editor View, Click Save Button or Ctrl+S\r\n\
Delete Snippet --- Select a snippet on SnippetDock and Click Delete Button\r\n\
Export Snippets --- Menu>Plugins>FingerText>Export Snippets\r\n\
Import Snippets --- Menu>Plugins>FingerText>Import Snippets\r\n\
About FingerText --- Menu>Plugins>FingerText>About\r\n\r\n\
For step by step usage guide, please visit http://github.com/erinata/FingerText \
")
#define SNIPPET_EDIT_TEMPLATE "------ FingerText Snippet Editor View ------\r\n"

#define WELCOME_TEXT "Thanks for Upgrading to FingerText 0.5.6.0(Alpha).\r\n\
Please read this document if you are upgrading from previous versions.\r\n\r\n\
Upgrading from 0.5.0 or above\r\n\
Everything is compatibile.\r\n\
Upgrading from 0.4.15 or 0.4.16\r\n\
If you want to use your snippets after you upgrade to 0.5.6, you can go to the config folder and move the FingerText.db3 file to the config\FingerText folder after update.\r\n\
chain snippet is indicated by $[![(cha)]!] instead of $[![(chain)]!]\r\n\
\r\n\
Upgrading from 0.4.4 or 0.4.11\r\n\
If you want to use your snippets after you upgrade to 0.5.6, you can go to the config folder and move the FingerText.db3 file to the config\FingerText folder after update.\r\n\
\r\n\
Upgrading from 0.4.1\r\n\
If you want to use your snippets after you upgrade to 0.5.6, you can go to the %Notepad++ folder%\plugins\FingerText and get the old database file Snippets.db3. Rename is to FingerText.db3 and move it to the config\FingerText folder after update.\r\n\
\r\n\
Upgrading from 0.3.5 or below\r\n\
FingerText 0.3.5 or below use a 'one snippet per file' system to store snippets, which is not compatibile with current version. If you really have a lot of snippets created using these early version, please send your snippet to erinata@gmail.com. I will try my best to import them into the database in the current version.\r\n\
"


#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        101
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
