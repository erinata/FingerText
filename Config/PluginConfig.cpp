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

#include "PluginConfig.h"

extern NppData nppData;

PluginConfig::PluginConfig()
{

}

void PluginConfig::setTextVariables()
{
    for (int i = 0; i < CONFIG_TEXT_COUNT ; i++)
    {
        configText[i] = new wchar_t[configTextLength[i]];
    }    
}

void PluginConfig::cleanTextVariables()
{
    for (int i = 0; i < CONFIG_TEXT_COUNT ; i++)
    {
        delete [] configText[i];
    }
}

void PluginConfig::loadConfigInt()
{

    for (int i = 0; i < CONFIG_INT_COUNT ; i++)
    {
        doLoadConfig(configInt[i],configIntNames[i],configIntDefault[i]);
    }


}

void PluginConfig::loadConfigText()
{

    for (int i = 0; i < CONFIG_TEXT_COUNT ; i++)
    {
        doLoadConfig(configText[i],configTextNames[i],configTextDefault[i],configTextLength[i]);
    }

}


void PluginConfig::writeConfigText()
{
    
    for (int i = 0; i < CONFIG_TEXT_COUNT ; i++)
    {
        doWriteConfig(configText[i],configTextNames[i]);
    }
}


void PluginConfig::writeConfigInt()
{

    for (int i = 0; i <CONFIG_INT_COUNT; i++)
    {
        doWriteConfig(configInt[i],configIntNames[i]);
    }

}







void PluginConfig::resetDefaultSettings()
{   

    for (int i = 0; i <CONFIG_INT_COUNT; i++)
    {
        configInt[i] = configIntDefault[i];
    }

    for (int i = 0; i < CONFIG_TEXT_COUNT ; i++)
    {
        wcscpy(configText[i],configTextDefault[i]);
    }
}


void PluginConfig::configSetUp()
{
    setTextVariables();

    version = ::GetPrivateProfileInt(TEXT(PLUGIN_NAME), TEXT("version"), 0, iniPath);
    
    if (version == VERSION_LINEAR)  // current version
    {
        loadConfigInt();
        loadConfigText();
        newUpdate = false;
        
    } else if ((version >= VERSION_KEEP_CONFIG_START) && (version <= VERSION_LINEAR))  // for version changes that do not want to reset ini path
    {
        version = VERSION_LINEAR;
        doWriteConfig(version,TEXT("version"));
                
        loadConfigInt();
        loadConfigText();
        writeConfigInt();
        writeConfigText();
        newUpdate = true;
    } else // for version that need ini file reset
    {
        version = VERSION_LINEAR;
        doWriteConfig(version,TEXT("version"));
        resetDefaultSettings();
        
        writeConfigInt();
        writeConfigText();
        newUpdate = true;
    }   
}

void PluginConfig::configCleanUp()
{
    cleanTextVariables();
}

void PluginConfig::doWriteConfig(int configInt, const wchar_t* section)
{
    wchar_t configText[32];
    _itow_s(configInt,configText, 10,10);
    ::WritePrivateProfileString(TEXT(PLUGIN_NAME), section, configText, iniPath);
}

void PluginConfig::doWriteConfig(wchar_t* configChar, const wchar_t* section)
{
    ::WritePrivateProfileString(TEXT(PLUGIN_NAME), section, configChar, iniPath);
}

void PluginConfig::doLoadConfig(int &variable, const TCHAR* key, const int defaultValue)
{
    variable = GetPrivateProfileInt(TEXT(PLUGIN_NAME), key, defaultValue, iniPath);
}

void PluginConfig::doLoadConfig(TCHAR* &variable, const TCHAR* key, const TCHAR* defaultValue, int length)
{
    GetPrivateProfileString(TEXT(PLUGIN_NAME), key, defaultValue,variable,length,iniPath);
}



void PluginConfig::callWriteConfigText(int index)
{
    if (index < 0 )
    {
        writeConfigText();
        
    } else
    {
        doWriteConfig(configText[index],configTextNames[index]);
    }
}

void PluginConfig::callWriteConfigInt(int index)
{
    if (index < 0 )
    {
        writeConfigInt();
        
    } else
    {
        doWriteConfig(configInt[index],configIntNames[index]);
    }
}

void PluginConfig::callLoadConfigText(int index)
{
    if (index < 0 )
    {
        loadConfigText();
        
    } else
    {
        doLoadConfig(configText[index],configTextNames[index],configTextDefault[index],configTextLength[index]);
    }
}

void PluginConfig::callLoadConfigInt(int index)
{
    if (index < 0 )
    {
        loadConfigInt();
        
    } else
    {
        doLoadConfig(configInt[index],configIntNames[index],configIntDefault[index]);
    }
}





void PluginConfig::upgradeMessage()
{
    if (newUpdate)
    {
    //TODO: make better structure for welcometext. like one piece of string per version.
    //TODO: dynamic upgrade message
        char* welcomeText = new char[10000];
        strcpy(welcomeText,"");

    
        strcat(welcomeText, "Thanks for Upgrading to ");
        strcat(welcomeText, PLUGIN_NAME);
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, " ");
        strcat(welcomeText, VERSION_STAGE);
        strcat(welcomeText, VERSION_STAGE_ADD);
        strcat(welcomeText, "\r\n\
Please read this document if you are upgrading from previous versions.\r\n\r\n\
Upgrading from 0.5.37 or above\r\n\
Everything is compatible.\r\n\r\n\
Upgrading from any version between 0.5.20 and 0.5.35\r\n\
In version ");
        strcat(welcomeText, VERSION_TEXT);
        strcat(welcomeText, " after you triggered a option hotspot, you can use right/down arrow to move to next option, left/up arrow to move to previous option. Hit tab confirm the choice.\r\n\r\n\
 Also the option hotspot is not delimited by |~| anymore, it's using is simplier |. So $[![(opt)ABC|DEF|GHI]!] will define an hotspot with 3 options.\r\n\
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
    

        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
        ::SendMessage(getCurrentScintilla(), SCI_INSERTTEXT, 0, (LPARAM)welcomeText);

        delete [] welcomeText;

    
    
    }
}



void PluginConfig::settings()
{
    // TODO: try putting settings into the ini files instead of just using annotation, or just use show tips as help. Showtips can be store in const array in the same system as config settings
    TCHAR* confirmationText = new TCHAR[wcslen(TEXT(PLUGIN_NAME))+200];
    ::wcscpy(confirmationText,TEXT("Change the settings only when you know what you are doing. Messing up the ini can cause "));
    ::wcscat(confirmationText,TEXT(PLUGIN_NAME));
    ::wcscat(confirmationText,TEXT(" to stop working.\r\n\r\n Do you wish to continue?"));

    if (showMessageBox(confirmationText, MB_YESNO) == IDYES)
    {
        callWriteConfigText();
        callWriteConfigInt();

        if (!::SendMessage(nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)iniPath))
        {
            ::SendMessage(nppData._nppHandle, NPPM_DOOPEN, 0, (LPARAM)iniPath);
        }
        int lineCount = ::SendScintilla(SCI_GETLINECOUNT, 0, 0)-1;
        ::SendScintilla(SCI_ANNOTATIONCLEARALL, 0, 0);
        //TODO: move this part (and other text) to const char in another file.
        ::SendScintilla(SCI_ANNOTATIONSETTEXT, lineCount, (LPARAM)"\
; This is the config file of FingerText.    \r\n\
; Do NOT mess up with the settings unless you know what you are doing \r\n\
; You need to restart Notepad++ to apply the changes\r\n\
; \r\n\
; version                    -- Don't change this\r\n\
; snippet_list_order_tagtype -- 0: The SnippetDock will order the snippets by trigger text\r\n\
;                               1: The SnippetDock will order the snippets by scope\r\n\
; indent_reference           -- 0: The snippnet content will be inserted without any change in\r\n\
;                                  indentation\r\n\
;                               1: The snippnet content will be inserted at the same indentation\r\n\
;                                  level as the trigger text\r\n\
; chain_limit                -- This is the maximum number dynamic hotspots that can be triggered in one\r\n\
;                               snippet. Default is 100\r\n\
; tab_tag_completion         -- 0: When a snippet is not found when the user hit [tab] key, FingerText\r\n\
;                                  will just send a tab\r\n\
;                               1: When a snippet is not found when the user hit [tab] key, FingerText\r\n\
;                                  will try to find the closest match snippet name\r\n\
; live_hint_update           -- 0: Turn off SnippetDock live update\r\n\
;                               1: Turn on SnippetDock live update\r\n\
; preserve_steps             -- Default is 0 and don't change it. It's for debugging use\r\n\
; import_overwrite_confirm   -- 0: Pop up confirmation box everytime you are about to overwrite a snippet.\r\n\
;                               1: No confirmation box when you overwrite a snippet.\r\n\
; inclusive_triggertext_completion -- 0: Tiggertext completion will only include triggertext which starts\r\n\
;                                        with the characters you are typing.\r\n\
;                                     1: Tiggertext completion will only include triggertext which\r\n\
;                                        includes the characters you are typing.\r\n\
; custom_scope               --  A user defined custom scope. For example if you put .rb here, you can use\r\n\
;                                all the .rb snippets in any files.\r\n\
; escape_char                --  Any text entered after this character will not be view as snippet. For\r\n\
;                                example if put <> here then you cannot trigger the snippet 'npp' by typing\r\n\
;                                either '<npp' or '>npp' and hit tab\r\n\
; live_preview_box           --  0: Turn off preview box live update\r\n\
;                                1: Turn on preview box live update\r\n\
; editor_caret_bound         --  0: Fingertext will not restrict caret movement in snippet editing mode.\r\n\
;                                   Do not set this to 0 unless you are very sure that you won't mess up\r\n\
;                                   the snippet editor format.\r\n\
;                            --  1: Fingertext will restrict caret movement in snippet editing mode.\r\n\
; force_multipaste           --  0: Use notepad++ settings to determine whether you can paste text into\r\n\
;                                   multiple hotspots simultaneously.\r\n\
;                            --  1: Force notepad++ to turn on multipasting feature.\r\n\
        ");
        ::SendScintilla(SCI_ANNOTATIONSETSTYLE, lineCount, STYLE_INDENTGUIDE);
        ::SendScintilla(SCI_ANNOTATIONSETVISIBLE, lineCount, 0);
    }

    delete [] confirmationText;
}




void PluginConfig::help()
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
    showMessageBox(helpText);
     //ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/erinata/FingerText"), NULL, NULL, SW_SHOWNORMAL);
}

void PluginConfig::about()
{
        
    int length = _tcslen(TEXT(PLUGIN_NAME))+_tcslen(TEXT(VERSION_TEXT))+_tcslen(TEXT(VERSION_STAGE))+_tcslen(TEXT(VERSION_STAGE_ADD))+_tcslen(TEXT(DATE_TEXT))+_tcslen(TEXT(AUTHOR_TEXT))+_tcslen(TEXT(EMAIL_TEXT))+_tcslen(TEXT(ABOUT_TEXT))+50;
   
    TCHAR* versionText = new TCHAR[length];
    
    _tcscpy(versionText,TEXT(""));
    _tcscat(versionText, TEXT(PLUGIN_NAME));
    _tcscat(versionText, TEXT(" "));
    _tcscat(versionText, TEXT(VERSION_TEXT));
    _tcscat(versionText, TEXT(VERSION_STAGE));
    _tcscat(versionText, TEXT(VERSION_STAGE_ADD));
    
    _tcscat(versionText,TEXT("\r\n"));
    _tcscat(versionText,TEXT(DATE_TEXT));

    _tcscat(versionText,TEXT("\r\n\r\n"));
    _tcscat(versionText,TEXT(AUTHOR_TEXT));
    _tcscat(versionText,TEXT(EMAIL_TEXT));

    _tcscat(versionText,TEXT("\r\n"));

    _tcscat(versionText,TEXT(ABOUT_TEXT));

    showMessageBox(versionText);

    delete [] versionText;
}