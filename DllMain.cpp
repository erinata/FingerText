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

extern FuncItem funcItem[MENU_LENGTH];
extern NppData nppData;
extern DockingDlg snippetDock;
extern WNDPROC	wndProcNpp;
extern int nppLoaded;
extern int sciFocus;
extern bool g_onHotSpot;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved)
{

    switch (reasonForCall)
    {
        case DLL_PROCESS_ATTACH:
            pluginInit(hModule);
            break;

        case DLL_PROCESS_DETACH:
	        commandMenuCleanUp();
            pluginCleanUp();
            break;
        //case DLL_THREAD_ATTACH:
        //    break;
        //case DLL_THREAD_DETACH:
        //    break;
    }

    return true;
}

LRESULT CALLBACK SubWndProcNpp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT	retVal = 0;
    switch (message)
    {
        case WM_COMMAND:
             if ((HIWORD(wParam) == SCEN_KILLFOCUS) && nppLoaded == 1)
             {
                 sciFocus = 0;
             } else if ((HIWORD(wParam) == SCEN_SETFOCUS) && nppLoaded == 1)
             {
                 sciFocus = 1;
             }
             retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
             break;
        case WM_CLOSE:
            closeNonSessionTabs(); // Doing this before the callwindowproc will prevent the ftb window to go into the session
            retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
            updateMode();  //Need to Do this because when a user attempt to close npp and the buffer shift to a file that's not saved, The bufferactivated message is not activated.
            break;
    	default:
			retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
			break;
    }
    return retVal;
}

//functions which are called by Notepad++
extern "C" __declspec(dllexport) void setInfo(NppData nppDataInfo)
{
	nppData = nppDataInfo;
    wndProcNpp = (WNDPROC)::SetWindowLongPtr(nppData._nppHandle, GWLP_WNDPROC, (LPARAM)SubWndProcNpp);
    dialogsInit();
    pathInit();
    configInit();
    dataBaseInit();
    commandMenuInit();
    variablesInit();
    
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return TEXT(PLUGIN_NAME);
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = MENU_LENGTH;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    switch(notifyCode->nmhdr.code)
    {
        //case NPPN_FILEOPENED:
        //    updateDockItems();
        //    updateMode();
        //    break;
        case NPPN_BUFFERACTIVATED:
            g_onHotSpot = false;
            updateMode();
            turnOffOptionMode();
            //refreshAnnotation();
            // No break here because NPPN_BUFFERACTIVATED also trigger updateDockItems();
        case NPPN_LANGCHANGED:
            
            //keyUpdate();
            //if (nppLoaded) updateDockItems();
            if (nppLoaded)
            {
                if (!snippetHintUpdate()) updateDockItems(true,false,"%",true);
            }
            //cleanOptionItem(); //This is not necessary........but the memory will keep a list of options used in last option dynamic hotspor call
            break;
            //TODO: Try to deal with repeated triggering of snippetHintUpdate and keyUpdateS
        case SCN_CHARADDED:
            g_onHotSpot = false;
            //alertNumber(notifyCode->ch);

            //refreshAnnotation();
            
            snippetHintUpdate();
            //turnOffOptionMode(); //TODO: verify that this is actually not needed
            //showPreview();
            break;

        case SCN_MODIFIED:
            
            // This can catch the to be inserted text, but it cannot catch tab as the text insert of tab is different in different situation
            //if ((nppLoaded) && (notifyCode->modificationType & (SC_MOD_BEFOREINSERT)))
            //{
            //    alertNumber(strcmp(notifyCode->text,"  "));
            //}
            //TODO: investigate better way to write this (may be use SC_MULTISTEPUNDOREDO and SC_LASTSTEPINUNDOREDO)
            if ((nppLoaded) & ((notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT | SC_LASTSTEPINUNDOREDO)) && (!(notifyCode->modificationType & (SC_MOD_CHANGESTYLE | SC_MOD_CHANGEFOLD)))))
            {
                //g_onHotSpot = false;
                //
                turnOffOptionMode();
                ::SendScintilla(SCI_AUTOCCANCEL,0,0);
                if (!(notifyCode->modificationType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO)))
                {


                    if (!(notifyCode->modificationType & (SC_MOD_INSERTTEXT))) snippetHintUpdate();
                    refreshAnnotation();
                }

                //if (notifyCode->modificationType & (SC_MOD_DELETETEXT)) g_onHotSpot = false;
             
            }
            break;
        case SCN_UPDATEUI:
            //g_onHotSpot = false;
            if (nppLoaded) selectionMonitor(notifyCode->updated);
            //if (notifyCode->updated & (SC_UPDATE_CONTENT))
            //{
            //    selectionMonitor(true);
            //       
            //} else
            //{
            //    selectionMonitor();
            //}
            break;
        case NPPN_FILESAVED:
            //keyUpdate();
            //refreshAnnotation();
            updateDockItems(true,false,"%",true);
            promptSaveSnippet();
            //snippetHintUpdate();
            
            break;
        case NPPN_READY:
            //initialize();
            nppLoaded = 1;
            nppReady();            
            break;
        case NPPN_SHORTCUTREMAPPED:
            shortCutRemapped();
            break;

        case NPPN_SHUTDOWN:
            
            pluginShutdown();
            break;

        case SCN_AUTOCSELECTION:
            g_onHotSpot = false;
            if (fingerTextListActive())
            {
                
                recordLastListItem(notifyCode->text);
                //::SendScintilla(SCI_INSERTTEXT,(WPARAM)notifyCode->position,(LPARAM)notifyCode->text);
                //::SendScintilla(SCI_AUTOCCANCEL,0,0);
                //hotSpotNavigation();
                doTabActivate(true);
            }
            break;
        case SCN_AUTOCCANCELLED:
            g_onHotSpot = false;
            fingerTextListActive();
            break;
        // TODO: consider using SC_MOD_CHANGEANNOTATION to shutdown use of annotation in snippet editing mode
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    // Npp Messages 

    //if (Message == WM_MOVE)
	//{
	//	::MessageBox(NULL, TEXT("MOVE"), TEXT("Trace"), MB_OK);
	//}


	return true;
}

extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return true;
}