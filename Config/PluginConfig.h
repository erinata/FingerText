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

#ifndef PLUGINCONFIG_H
#define PLUGINCONFIG_H

#include "PluginDefinition.h"
//#include <windows.h>
//#include "Version.h"

// Config file content
#define CONFIG_INT_COUNT 15
#define CONFIG_TEXT_COUNT 3

#define SNIPPET_LIST_ORDER_TAG_TYPE 0
#define TAB_TAG_COMPLETION 1
#define LIVE_HINT_UPDATE 2
#define INDENT_REFERENCE 3
#define CHAIN_LIMIT 4
#define PRESERVE_STEPS 5
#define IMPORT_OVERWRITE_CONFIRM 6
#define INCLUSIVE_TRIGGERTEXT_COMPLETION 7
#define LIVE_PREVIEW_BOX 8
#define EDITOR_CARET_BOUND 9
#define FORCE_MULTI_PASTE 10
#define USE_NPP_SHORTKEY 11
#define FALLBACK_TAB 12
#define INSERTION_DIALOG_STATE 13
#define INSERT_MODE 14


#define CUSTOM_SCOPE 0
#define CUSTOM_ESCAPE_CHAR 1
#define PARAMS_DELIMITER 2

const int configIntDefault[CONFIG_INT_COUNT] = 
{
    1,               // SNIPPET_LIST_ORDER_TAG_TYPE
    0,               // TAB_TAG_COMPLETION
    1,               // LIVE_HINT_UPDATE
    1,               // INDENT_REFERENCE
    100,             // CHAIN_LIMIT
    0,               // PRESERVE_STEPS
    0,               // IMPORT_OVERWRITE_CONFIRM
    0,               // INCLUSIVE_TRIGGERTEXT_COMPLETION
    1,               // LIVE_PREVIEW_BOX
    1,               // EDITOR_CARET_BOUND
    1,               // FORCE_MULTI_PASTE
    1,               // USE_NPP_SHORTKEY
    0,               // FALLBACK_TAB
    0,               // INSERTION_DIALOG_STATE
    1                // INSERT_MODE
    

};                   

const TCHAR configIntNames[CONFIG_INT_COUNT][MAX_PATH] = 
{
    TEXT("snippet_list_order_tagtype"),
    TEXT("tab_tag_completion"),
    TEXT("live_hint_update"),
    TEXT("indent_reference"),
    TEXT("chain_limit"),
    TEXT("preserve_steps"),
    TEXT("import_overwrite_confirm"),
    TEXT("inclusive_triggertext_completion"),
    TEXT("live_preview_box"),
    TEXT("editor_caret_bound"),
    TEXT("force_multipaste"),
    TEXT("use_npp_shortkey"),
    TEXT("fallback_tab"),
    TEXT("insertion_dialog_state"),
    TEXT("insert_mode")
    
};

const TCHAR configTextDefault[CONFIG_TEXT_COUNT][MAX_PATH] = 
{
    TEXT(""),          //CUSTOM_SCOPE
    TEXT(""),          //CUSTOM_ESCAPE_CHAR
    TEXT(",")          //PARAMS_DELIMITER
};

const TCHAR configTextNames[CONFIG_TEXT_COUNT][MAX_PATH] = 
{
    TEXT("custom_scope"),
    TEXT("escape_char"),
    TEXT("params_delimiter")
};


const int configTextLength[CONFIG_TEXT_COUNT] = 
{
    MAX_PATH,            //CUSTOM_SCOPE
    MAX_PATH,            //CUSTOM_ESCAPE_CHAR
    2                    //PARAMS_DELIMITER
};


class PluginConfig
{
     //TODO: lazy loading of config.....
    public:
        PluginConfig::PluginConfig();
        void PluginConfig::configSetUp();
        void PluginConfig::configCleanUp();
        
        void PluginConfig::callWriteConfigText(int index = -1);
        void PluginConfig::callWriteConfigInt(int index = -1);
        void PluginConfig::callLoadConfigText(int index = -1);
        void PluginConfig::callLoadConfigInt(int index = -1);        

        void PluginConfig::upgradeMessage();
        void PluginConfig::settings();
        void PluginConfig::help();
        void PluginConfig::about();

        // location of the ini files
        wchar_t iniPath[MAX_PATH];

        // Core Variables 
        int version;
        int versionOld;
        bool newUpdate;

        // variable from ini file
        int configInt[CONFIG_INT_COUNT];
        TCHAR* configText[CONFIG_TEXT_COUNT];

    private:
        void PluginConfig::setTextVariables();
        void PluginConfig::cleanTextVariables();

        void PluginConfig::loadConfigInt();
        void PluginConfig::loadConfigText();
        void PluginConfig::writeConfigInt();
        void PluginConfig::writeConfigText();
        void PluginConfig::resetDefaultSettings();
        
        void PluginConfig::doWriteConfig(wchar_t* configChar, const wchar_t* section);
        void PluginConfig::doWriteConfig(int configInt, const wchar_t* section);       
        void PluginConfig::doLoadConfig(int &variable, const TCHAR* key, const int defaultValue);
        void PluginConfig::doLoadConfig(TCHAR* &variable, const TCHAR* key, const TCHAR* defaultValue, int length);
};



#endif //PLUGINCONFIG_H