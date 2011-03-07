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

#include "PluginDefinition.h"

#include "SnippetDock.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern DockingDlg snippetDock;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  reasonForCall, 
                       LPVOID lpReserved )
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

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
    setConfigAndDatabase();
    
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
        case NPPN_READY:
            upgradeMessage();
            initialize();
            break;
        case NPPN_SHUTDOWN:
            pluginShutdown();
            break;
        //case NPPN_FILEOPENED:
        //    updateDockItems();
        //    updateMode();
        //    break;
        case NPPN_BUFFERACTIVATED:
            // TODO: there may be some problem using updateMode here, as the buffere activate is not fired when the focus is switch to the ftb file when a use try to close npp and cancel afterwards.
            updateMode();
            //keyUpdate();
            refreshAnnotation();
            updateDockItems();
            break;
        
            //TODO: Try to deal with repeated triggering of snippetHintUpdate and keyUpdate
        case SCN_KEY:
            alert();
            break;
        case SCN_CHARADDED:

            //switch(notifyCode->ch)
            //{
            
            //}
            
            //keyUpdate();
            //refreshAnnotation();
            //break;   // should also do snippetHintUpdate() when SCN_CHARADDED
            snippetHintUpdate();
            //showPreview();
            break;
        case SCN_MODIFIED:
            //alert();
            //if (notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_MOD_INSERTTEXT))
            if (notifyCode->modificationType & (SC_MOD_DELETETEXT | SC_LASTSTEPINUNDOREDO))
            {
                
                snippetHintUpdate();
                refreshAnnotation();
            }
            break;
        
        case NPPN_FILESAVED:
            //keyUpdate();
            
            //refreshAnnotation();
            promptSaveSnippet();
            //snippetHintUpdate();
            updateDockItems();
            break;
        
    }
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{


	//if (Message == WM_MOVE)
	//{
	//	::MessageBox(NULL, TEXT("move"), TEXT(""), MB_OK);
	//}

	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
