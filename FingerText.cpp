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
#include "SnippetDock.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern DockingDlg snippetDock;
WNDPROC	wndProcNpp = NULL;

BOOL APIENTRY DllMain( HANDLE hModule, DWORD reasonForCall, LPVOID lpReserved )
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
        case WM_CLOSE:
            //alert();
            retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
            updateMode();  //Need to Do this because then a user attempt to close npp and the buffer shift to a file that's not saved, The bufferactivated message is not activated.
            
            break;
    	default:
			retVal = ::CallWindowProc(wndProcNpp, hWnd, message, wParam, lParam);
			break;
    }
    return retVal;
}

//functions which are called by Notepad++ plugin manager
extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
    wndProcNpp = (WNDPROC)::SetWindowLongPtr(nppData._nppHandle, GWL_WNDPROC, (LPARAM)SubWndProcNpp);
	commandMenuInit();
    initialize();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
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
            updateDockItems();
            //cleanOptionItem(); //This is not necessary........but the memory will keep a list of options used in last option dynamic hotspor call
            break;
            //TODO: Try to deal with repeated triggering of snippetHintUpdate and keyUpdate
        //case SCN_KEY:
        //    //alert();
        //    break;
        case SCN_CHARADDED:
            //switch(notifyCode->ch)
            //{
            //}
            //keyUpdate();
            //refreshAnnotation();
            //break;   
            snippetHintUpdate();
            turnOffOptionMode();
            //showPreview();
            break;

        case SCN_MODIFIED:
            //if (notifyCode->modificationType & SC_MOD_BEFOREINSERT)
            //{
            //    alert();
            //    if (strlen(notifyCode->text)>1) alert();
            //    
            //}
            //if (notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT))
            
            //TODO: investigate better way to write this (may be use SC_MULTISTEPUNDOREDO and SC_LASTSTEPINUNDOREDO)
            if ((notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT | SC_LASTSTEPINUNDOREDO)) && (!(notifyCode->modificationType & (SC_PERFORMED_UNDO | SC_PERFORMED_REDO))))
            {
                turnOffOptionMode();
                snippetHintUpdate();
                refreshAnnotation();
            }
            break;
        case SCN_UPDATEUI:        
            selectionMonitor(notifyCode->updated);
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
            upgradeMessage();
            
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