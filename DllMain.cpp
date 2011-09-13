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
WNDPROC	wndProcNpp = NULL;
int nppLoaded = 0;
HINSTANCE hinstance = NULL;
HHOOK hook = NULL;
HWND hwnd = NULL;

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

    hinstance = (HINSTANCE)hModule;
    return true;
}

LRESULT CALLBACK SubWndProcNpp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    LRESULT	retVal = 0;
    switch (message)
    {
        case WM_CLOSE:
            retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
            updateMode();  //Need to Do this because when a user attempt to close npp and the buffer shift to a file that's not saved, The bufferactivated message is not activated.
            break;
    	default:
			retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
			break;
    }
    return retVal;
}




LRESULT CALLBACK hookproc(int ncode,WPARAM wparam,LPARAM lparam)
{
	if(ncode>=0)
	{
		if((lparam & 0x80000000) == 0x00000000)//Check whether key was pressed(not released).
		{
            alert();
			//hwnd = FindWindow("#32770","Keylogger Exe");//Find application window handle
			//PostMessage(hwnd,WM_USER+755,wparam,lparam);//Send info to app Window.
		}
	}
	return ( CallNextHookEx(hook,ncode,wparam,lparam) );//pass control to next hook in the hook chain.
}



void installhook(HWND h)
{
	hook = NULL;
	hwnd = h;
	hook = SetWindowsHookEx(WH_KEYBOARD,hookproc,hinstance,NULL);
	if(hook==NULL)
		MessageBox(NULL,TEXT("Unable to install hook"),TEXT("Error!"),MB_OK);
}

void removehook()
{
	UnhookWindowsHookEx(hook);
}




//functions which are called by Notepad++
extern "C" __declspec(dllexport) void setInfo(NppData nppDataInfo)
{
	nppData = nppDataInfo;
    wndProcNpp = (WNDPROC)::SetWindowLongPtr(nppData._nppHandle, GWL_WNDPROC, (LPARAM)SubWndProcNpp);
	commandMenuInit();
    dialogsInit();
    pathInit();
    configInit();
    dataBaseInit();
    variablesInit();
    installhook(nppData._nppHandle);
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
            updateMode();
            turnOffOptionMode();
            //refreshAnnotation();
            // No break here because NPPN_BUFFERACTIVATED also trigger updateDockItems();
        case NPPN_LANGCHANGED:
            
            //keyUpdate();
            if (nppLoaded) updateDockItems();
            //cleanOptionItem(); //This is not necessary........but the memory will keep a list of options used in last option dynamic hotspor call
            break;
            //TODO: Try to deal with repeated triggering of snippetHintUpdate and keyUpdateS
        case SCN_CHARADDED:
            
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
            if ((nppLoaded) & ((notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT | SC_LASTSTEPINUNDOREDO)) && (!(notifyCode->modificationType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO)))))
            {
                
                turnOffOptionMode();
                if (!(notifyCode->modificationType & (SC_MOD_INSERTTEXT))) snippetHintUpdate();
                refreshAnnotation();
             
            }
            break;
        case SCN_UPDATEUI:  
            
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
            promptSaveSnippet();
            //snippetHintUpdate();
            updateDockItems();
            break;

        case NPPN_READY:
            //initialize();
            nppLoaded = 1;
            nppReady();
            

            break;

        case NPPN_SHUTDOWN:
            pluginShutdown();
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